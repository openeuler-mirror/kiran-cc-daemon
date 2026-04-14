/**
 * Copyright (c) 2020 ~ 2026 KylinSec Co., Ltd.
 * kiran-cc-daemon is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:    gaobo <gaobo@kylinsec.com.cn>
 */

#include <gio/gunixmounts.h>
#include <glib.h>

#include "disk-space-helper.h"
#include "disk-space-manager.h"
#include "low-disk-space-notifier.h"

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QGSettings>
#include <QProcess>
#include <QTimer>
#include <qt5-log-i.h>

namespace Kiran
{

namespace
{
/*
 * 当前未实现项：
 * 1) multiple_volumes：仅保留统计日志（暂不区分“本机/卷名”文案分支）。
 * 2) other_usable_volumes：仅保留统计日志，未传入对话框参与 UI 行为。
 */

/* mate-settings-daemon msd-disk-space.c ldsm_mount_should_ignore: ignore_fs */
const char* const kIgnoreFs[] = {"adfs",    "afs",     "auto",   "autofs",   "autofs4", "cifs",    "cxfs",
                                 "devfs",   "devpts",  "ecryptfs", "fdescfs", "gfs",     "gfs2",    "kernfs",
                                 "linprocfs", "linsysfs", "lustre", "lustre_lite", "ncpfs", "nfs", "nfs4",
                                 "nfsd",    "ocfs2",   "proc",   "procfs",   "ptyfs",   "rpc_pipefs", "selinuxfs",
                                 "smbfs",   "sysfs",   "tmpfs",  "usbfs",    "zfs",     nullptr};

/* ignore_devices */
const char* const kIgnoreDevices[] = {"none", "sunrpc", "devpts", "nfsd", "/dev/loop", "/dev/vn", nullptr};

const char* const kDiskAnalyzerProgram = "mate-disk-usage-analyzer";
constexpr int kMountChangeThrottleMs = 5000;

}  // namespace

DiskSpaceManager* DiskSpaceManager::s_instance = nullptr;

void DiskSpaceManager::globalInit()
{
    if (s_instance)
        return;
    s_instance = new DiskSpaceManager(qApp);
}

DiskSpaceManager::DiskSpaceManager(QObject* parent) : QObject(parent)
{
    m_settings = new QGSettings(QByteArrayLiteral("com.kylinsec.kiran.disk-space"), QByteArray(), this);

    m_timer = new QTimer(this);
    m_timer->setSingleShot(false);
    m_mountChangeThrottleTimer = new QTimer(this);
    m_mountChangeThrottleTimer->setSingleShot(true);
    connect(m_timer, &QTimer::timeout, this, &DiskSpaceManager::onTimerTick);
    connect(m_mountChangeThrottleTimer, &QTimer::timeout, this, &DiskSpaceManager::onMountChangeThrottleTimeout);
    connect(m_settings, &QGSettings::changed, this, &DiskSpaceManager::onSettingsChanged);

    m_lowDiskNotifier = new LowDiskSpaceNotifier(this);
    connect(m_lowDiskNotifier, &LowDiskSpaceNotifier::response, this, &DiskSpaceManager::onLowDiskNotifyResponse,
            Qt::QueuedConnection);

    reloadSettings();
    start();
}

DiskSpaceManager::~DiskSpaceManager()
{
    stop();
    if (s_instance == this)
        s_instance = nullptr;
}

void DiskSpaceManager::reloadSettings()
{
    {
        const double raw = m_settings->get(QStringLiteral("free-percent-notify")).toDouble();
        double norm = 0.05;
        if (!DiskSpaceHelper::normalizeFreePercentInOpen01(raw, 0.05, &norm))
            KLOG_WARNING() << "disk-space: invalid free-percent-notify, fallback 0.05";
        m_freePercentNotify = norm;
    }
    {
        const double raw = m_settings->get(QStringLiteral("free-percent-notify-again")).toDouble();
        double norm = 0.01;
        if (!DiskSpaceHelper::normalizeFreePercentInOpen01(raw, 0.01, &norm))
            KLOG_WARNING() << "disk-space: invalid free-percent-notify-again, fallback 0.01";
        m_freePercentNotifyAgain = norm;
    }
    m_freeSizeGbNoNotify = m_settings->get(QStringLiteral("free-size-gb-no-notify")).toInt();
    // 两次真实提示（通知）的最小间隔（分）；仅作用于已 notify 过且满足 notify-again 条件时，见 processNextWarningMount
    m_minNotifyPeriodMinutes = m_settings->get(QStringLiteral("min-notify-period")).toInt();
    m_ignorePaths = m_settings->get(QStringLiteral("ignore-paths")).toStringList();
    m_checkIntervalSec = m_settings->get(QStringLiteral("check-interval-seconds")).toInt();
    if (m_checkIntervalSec < 10)
    {
        KLOG_WARNING() << "disk-space: invalid check-interval-seconds, fallback 60";
        m_checkIntervalSec = 60;
    }

    /* 路径新进入 ignore-paths 时，从已通知状态中剔除，避免死记录 */
    QHash<QString, NotifyRecord> kept = m_notified;
    m_notified.clear();
    auto it = kept.constBegin();
    for (; it != kept.constEnd(); ++it)
    {
        if (!DiskSpaceHelper::mountPathInIgnoreList(it.key(), m_ignorePaths))
            m_notified.insert(it.key(), it.value());
    }

    if (m_timer && m_timer->isActive())
        m_timer->setInterval(m_checkIntervalSec * 1000);

    KLOG_INFO() << "disk-space: settings reloaded, notify=" << m_freePercentNotify
                << "notify-again=" << m_freePercentNotifyAgain
                << "gb-no-notify=" << m_freeSizeGbNoNotify
                << "min-notify-min=" << m_minNotifyPeriodMinutes
                << "ignore-count=" << m_ignorePaths.size()
                << "interval-sec=" << m_checkIntervalSec;
}

void DiskSpaceManager::start()
{
    KLOG_INFO() << "disk-space: start monitor, interval-sec=" << m_checkIntervalSec;

    m_mountMonitor = g_unix_mount_monitor_get();
    g_signal_connect(m_mountMonitor, "mounts-changed",
                     G_CALLBACK(onMountsChangedCallback), this);

    m_timer->start(m_checkIntervalSec * 1000);
    QTimer::singleShot(0, this, &DiskSpaceManager::checkAllMounts);
}

void DiskSpaceManager::stop()
{
    KLOG_INFO() << "disk-space: stop monitor";
    m_timer->stop();
    if (m_mountChangeThrottleTimer)
        m_mountChangeThrottleTimer->stop();
    m_mountChangeThrottled = false;

    if (m_mountMonitor)
    {
        g_signal_handlers_disconnect_by_data(m_mountMonitor, this);
        g_object_unref(m_mountMonitor);
        m_mountMonitor = nullptr;
    }

    m_notified.clear();
    m_pendingMounts.clear();
}

bool DiskSpaceManager::mountHasSpace(const MountSnap& m) const
{
    return DiskSpaceHelper::mountHasEnoughSpace(m.buf, m_freePercentNotify, m_freeSizeGbNoNotify);
}

void DiskSpaceManager::onSettingsChanged(const QString& /* key */)
{
    reloadSettings();
    checkAllMounts();
}

void DiskSpaceManager::onTimerTick()
{
    checkAllMounts();
}

void DiskSpaceManager::checkAllMounts()
{
    // 活动通知存在时不并发第二条、不改写队列；通知关闭路径不调用本函数补扫（仅定时器与 mounts-changed）。
    if (m_lowDiskNotifier && m_lowDiskNotifier->hasActiveNotification())
    {
        KLOG_DEBUG() << "disk-space: skip check, low-disk notification still active";
        return;
    }

    QVector<MountSnap> checkMounts = collectMounts();
    QSet<QString> present;
    for (const MountSnap& m : checkMounts)
        present.insert(m.path);
    pruneNotifiedByMounts(present);

    const int numberOfMounts = checkMounts.size();
    const bool multipleVolumes = numberOfMounts > 1;

    m_pendingMounts.clear();
    // 仍低于阈值进 m_pendingMounts；已恢复 enough space 的卷从 m_notified 移除
    for (const MountSnap& mm : checkMounts)
    {
        if (mountHasSpace(mm))
        {
            m_notified.remove(mm.path);
            continue;
        }
        m_pendingMounts.append(mm);
    }

    bool otherUsable = numberOfMounts > m_pendingMounts.size();
    KLOG_DEBUG() << "disk-space: check result, mounts=" << numberOfMounts
                 << "multiple-volumes=" << multipleVolumes
                 << "low-space=" << m_pendingMounts.size()
                 << "other-usable=" << otherUsable;
    m_nextPendingIndex = 0;
    processNextWarningMount();
}


DiskSpaceHelper::NotifyCheckInput DiskSpaceManager::makeNotifyCheckInput(const MountSnap& mount,
                                                                         bool hasPrevRecord,
                                                                         const NotifyRecord& prevRecord) const
{
    DiskSpaceHelper::NotifyCheckInput checkInput;
    checkInput.hasRecord = hasPrevRecord;
    checkInput.lastNotifyTime = hasPrevRecord ? prevRecord.notifyTime : 0;
    checkInput.lastBuf = {};
    if (hasPrevRecord)
        checkInput.lastBuf = prevRecord.lastBuf;
    checkInput.currentBuf = mount.buf;
    checkInput.currentTime = time(nullptr);
    checkInput.notifyAgainThreshold = m_freePercentNotifyAgain;
    checkInput.minNotifyPeriodMinutes = m_minNotifyPeriodMinutes;
    return checkInput;
}

void DiskSpaceManager::updateNotifyRecord(const MountSnap& mountInfo, const DiskSpaceHelper::NotifyCheckResult& checkResult)
{
    const QString& path = mountInfo.path;

    if (checkResult.updateRecord)
    {
        NotifyRecord nr;
        nr.notifyTime = checkResult.newNotifyTime;
        nr.lastBuf = checkResult.newSnapshot;
        m_notified.insert(path, nr);
    }
    else
    {
        KLOG_DEBUG() << "disk-space: suppress notify by free-percent-notify-again for mount" << path;
    }
}

void DiskSpaceManager::openLowDiskSpaceNotification(const MountSnap& mountInfo)
{
    const QString path = mountInfo.path;
    const bool hasAnalyzer = progInPath(QString::fromLatin1(kDiskAnalyzerProgram));
    qint64 freeBytes = static_cast<qint64>(mountInfo.buf.f_frsize) * static_cast<qint64>(mountInfo.buf.f_bavail);

    KLOG_INFO() << "disk-space: show low-disk notification mount=" << path << "analyzer=" << hasAnalyzer;

    if (!m_lowDiskNotifier ||
        !m_lowDiskNotifier->show(hasAnalyzer, freeBytes, mountInfo.displayName, mountInfo.path))
    {
        KLOG_WARNING() << "disk-space: notification not shown for mount" << path;
        ++m_nextPendingIndex;
        processNextWarningMount();
    }
}

void DiskSpaceManager::processNextWarningMount()
{
    if (m_lowDiskNotifier && m_lowDiskNotifier->hasActiveNotification())
        return;

    while (m_nextPendingIndex < m_pendingMounts.size())
    {
        const MountSnap& mountInfo = m_pendingMounts[m_nextPendingIndex];
        const QString path = mountInfo.path;

        const bool hasPrevRecord = m_notified.contains(path);
        NotifyRecord prevRecord;
        if (hasPrevRecord)
            prevRecord = m_notified[path];

        const DiskSpaceHelper::NotifyCheckInput checkInput =
            makeNotifyCheckInput(mountInfo, hasPrevRecord, prevRecord);
        const DiskSpaceHelper::NotifyCheckResult checkResult = DiskSpaceHelper::checkShouldNotify(checkInput);

        updateNotifyRecord(mountInfo, checkResult);

        if (!checkResult.shouldNotify)
        {
            ++m_nextPendingIndex;
            continue;
        }

        openLowDiskSpaceNotification(mountInfo);
        return;
    }

    m_pendingMounts.clear();
    m_nextPendingIndex = 0;
}

void DiskSpaceManager::onLowDiskNotifyResponse(bool launchedAnalyzer, const QString& actedMountPath)
{
    if (!m_lowDiskNotifier)
        return;

    KLOG_INFO() << "disk-space: low-disk notification response mount=" << actedMountPath
                << "launch-analyzer=" << launchedAnalyzer;

    if (launchedAnalyzer)
        DiskSpaceManager::launchDiskAnalyzer(actedMountPath);

    ++m_nextPendingIndex;
    processNextWarningMount();
}

bool DiskSpaceManager::mountShouldIgnore(const QString& path, const QString& fsType, const QString& devicePath) const
{
    if (DiskSpaceHelper::mountPathInIgnoreList(path, m_ignorePaths))
    {
        KLOG_DEBUG() << "disk-space: ignore mount by ignore-paths" << path;
        return true;
    }
    if (DiskSpaceHelper::isInSet(fsType.toLatin1(), kIgnoreFs))
    {
        KLOG_DEBUG() << "disk-space: ignore mount by fs-type" << path << fsType;
        return true;
    }
    if (DiskSpaceHelper::isInSet(devicePath.toLatin1(), kIgnoreDevices))
    {
        KLOG_DEBUG() << "disk-space: ignore mount by device" << path << devicePath;
        return true;
    }
    return false;
}

QVector<MountSnap> DiskSpaceManager::collectMounts() const
{
    /*
     * 同步枚举挂载点（GIO）：先收集「有挂载条目且可写」的候选（path/fs/device），
     * 再对候选做 ignore、guess_name、statvfs；虚拟文件系统由 mountIsVirtual 过滤。
     */
    struct WritableUnixMount
    {
        QString path;
        QString fsType;
        QString devicePath;
    };

    QVector<WritableUnixMount> candidates;
    GList* points = g_unix_mount_points_get(nullptr);
    for (GList* l = points; l; l = l->next)
    {
        auto* mp = static_cast<GUnixMountPoint*>(l->data);
        const gchar* pathC = g_unix_mount_point_get_mount_path(mp);
        GUnixMountEntry* mount = g_unix_mount_at(pathC, nullptr);
        g_unix_mount_point_free(mp);
        if (!mount)
            continue;

        if (g_unix_mount_is_readonly(mount))
        {
            g_unix_mount_free(mount);
            continue;
        }

        WritableUnixMount c;
        c.path = QString::fromUtf8(g_unix_mount_get_mount_path(mount));
        c.fsType = QString::fromUtf8(g_unix_mount_get_fs_type(mount));
        c.devicePath = QString::fromUtf8(g_unix_mount_get_device_path(mount));
        g_unix_mount_free(mount);
        candidates.append(c);
    }
    g_list_free(points);

    QVector<MountSnap> result;
    for (const WritableUnixMount& c : candidates)
    {
        if (mountShouldIgnore(c.path, c.fsType, c.devicePath))
            continue;

        GUnixMountEntry* mount = g_unix_mount_at(c.path.toUtf8().constData(), nullptr);
        if (!mount)
            continue;

        MountSnap snap;
        snap.path = c.path;
        gchar* nameGuess = g_unix_mount_guess_name(mount);
        snap.displayName = nameGuess ? QString::fromUtf8(nameGuess) : c.path;
        g_free(nameGuess);
        const QByteArray pathUtf8 = c.path.toUtf8();
        if (statvfs(pathUtf8.constData(), &snap.buf) != 0)
        {
            KLOG_WARNING() << "disk-space: statvfs failed for mount" << c.path;
            g_unix_mount_free(mount);
            continue;
        }
        g_unix_mount_free(mount);

        if (DiskSpaceHelper::mountIsVirtual(snap.buf))
            continue;
        result.append(snap);
    }
    return result;
}

void DiskSpaceManager::pruneNotifiedByMounts(const QSet<QString>& presentMounts)
{
    // 已卸载的卷从 m_notified 删除，避免路径消失后状态残留
    int removed = 0;
    auto it = m_notified.begin();
    while (it != m_notified.end())
    {
        if (!presentMounts.contains(it.key()))
        {
            it = m_notified.erase(it);
            ++removed;
        }
        else
        {
            ++it;
        }
    }
    if (removed > 0)
        KLOG_DEBUG() << "disk-space: pruned notified entries" << removed;
}

bool DiskSpaceManager::progInPath(const QString& prog)
{
    if (prog.contains(QLatin1Char('/')))
        return QFileInfo::exists(prog);
    const QString pathEnv = QString::fromLocal8Bit(qgetenv("PATH"));
    for (const QString& segment : pathEnv.split(QLatin1Char(':')))
    {
        if (segment.isEmpty())
            continue;
        if (QFileInfo::exists(QDir(segment).absoluteFilePath(prog)))
            return true;
    }
    return false;
}

void DiskSpaceManager::launchDiskAnalyzer(const QString& mountPath)
{
    const QString program = QString::fromLatin1(kDiskAnalyzerProgram);
    const QStringList args{mountPath};

    KLOG_INFO() << "disk-space: launch analyzer program" << mountPath;
    const bool ok = QProcess::startDetached(program, args);
    if (!ok)
        KLOG_WARNING() << "disk-space: failed to launch analyzer program" << program << "args" << args;
}

void DiskSpaceManager::onMountsChangedCallback(GObject* /*monitor*/, gpointer userData)
{
    auto* self = static_cast<DiskSpaceManager*>(userData);
    if (self)
        self->onMountsChanged();
}

void DiskSpaceManager::onMountsChanged()
{
    KLOG_DEBUG() << "disk-space: mounts changed signal received";
    if (m_mountChangeThrottled)
    {
        KLOG_DEBUG() << "disk-space: mounts-changed ignored by throttle";
        return;
    }

    m_mountChangeThrottled = true;
    if (m_mountChangeThrottleTimer)
        m_mountChangeThrottleTimer->start(kMountChangeThrottleMs);
    KLOG_DEBUG() << "disk-space: mounts-changed accepted, throttle started"
                 << kMountChangeThrottleMs << "ms";

    // 低空间通知展示期间跳过，避免重入问题
    if (m_lowDiskNotifier && m_lowDiskNotifier->hasActiveNotification())
    {
        KLOG_DEBUG() << "disk-space: skip mounts-changed, low-disk notification active";
        return;
    }

    // 清理已卸载卷的通知记录
    QVector<MountSnap> current = collectMounts();
    QSet<QString> present;
    for (const auto& m : current)
        present.insert(m.path);
    pruneNotifiedByMounts(present);

    // 立即检查所有挂载点
    checkAllMounts();

    // 重置定时器
    m_timer->start(m_checkIntervalSec * 1000);
}

void DiskSpaceManager::onMountChangeThrottleTimeout()
{
    m_mountChangeThrottled = false;
    KLOG_DEBUG() << "disk-space: mounts-changed throttle window ended";
}

}  // namespace Kiran
