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

#pragma once

#include "disk-space-helper.h"

#include <QHash>
#include <QObject>
#include <QSet>
#include <QString>
#include <QVector>
#include <sys/statvfs.h>
#include <time.h>

class QGSettings;
class QTimer;
typedef struct _GObject GObject;
typedef void* gpointer;
typedef struct _GUnixMountMonitor GUnixMountMonitor;

namespace Kiran
{
class LowDiskSpaceNotifier;

struct MountSnap
{
    QString path;
    QString displayName;
    struct statvfs buf {};
};

struct NotifyRecord
{
    time_t notifyTime = 0;
    struct statvfs lastBuf {};
};

class DiskSpaceManager : public QObject
{
    Q_OBJECT

public:
    static DiskSpaceManager* getInstance() { return s_instance; }
    // 构造单例并启动监控；重复调用无效。
    static void globalInit();
    static void globalDeinit() { delete s_instance; }

private:
    explicit DiskSpaceManager(QObject* parent = nullptr);
    ~DiskSpaceManager() override;

    // 连接 GUnixMountMonitor、启动周期定时器并立即跑一次检查。
    void start();
    // 断开挂载监听、停表并清空队列与通知状态
    void stop();

    // 从 GSettings 读阈值与忽略列表，同步 m_notified 与定时器间隔。
    void reloadSettings();

    // 当前 statvfs 是否仍高于告警阈值（含 GiB 豁免）。
    bool mountHasSpace(const MountSnap& m) const;

    // 枚举可写静态挂载点并 statvfs，应用 ignore 与虚拟类型过滤。
    QVector<MountSnap> collectMounts() const;

    // 从 m_notified 中删除已不在 presentMounts 中的路径（卸载后不留脏状态）。
    void pruneNotifiedByMounts(const QSet<QString>& presentMounts);

    // ignore-paths、文件系统类型、设备路径任一命中则跳过该挂载。
    bool mountShouldIgnore(const QString& path, const QString& fsType, const QString& devicePath) const;

    // 可执行名在 PATH 中是否存在（或绝对路径存在）。
    static bool progInPath(const QString& prog);

    // 固定启动 mate-disk-usage-analyzer，参数为告警卷挂载路径。
    static void launchDiskAnalyzer(const QString& mountPath);

    // 一轮扫描：更新 present、修剪 m_notified；将不足空间卷装入告警队列并顺序处理。
    void checkAllMounts();

    // 处理告警队列中下一项：节流判定后或展示低空间通知或跳过。
    void processNextWarningMount();

    DiskSpaceHelper::NotifyCheckInput makeNotifyCheckInput(const MountSnap& mount, bool hasPrevRecord,
                                                           const NotifyRecord& prevRecord) const;
    // 按 notify 判定结果更新 m_notified（若 updateRecord）；未更新记录时打一条 DEBUG 说明原因。
    void updateNotifyRecord(const MountSnap& mountInfo, const DiskSpaceHelper::NotifyCheckResult& result);
    void openLowDiskSpaceNotification(const MountSnap& mountInfo);

private slots:
    void onSettingsChanged(const QString& /* key */);
    void onTimerTick();
    // GIO mounts-changed 后的槽：5s 前沿节流（首次立即执行，窗口期内忽略）。
    void onMountsChanged();
    void onMountChangeThrottleTimeout();
    void onLowDiskNotifyResponse(bool launchedAnalyzer, const QString& mountPath);

private:
    // GUnixMountMonitor 的 C 回调，仅转发到 onMountsChanged()。
    static void onMountsChangedCallback(GObject* monitor, gpointer userData);
    static DiskSpaceManager* s_instance;

    QGSettings* m_settings = nullptr;
    QTimer* m_timer = nullptr;
    LowDiskSpaceNotifier* m_lowDiskNotifier = nullptr;
    GUnixMountMonitor* m_mountMonitor = nullptr;

    double m_freePercentNotify = 0.05;
    double m_freePercentNotifyAgain = 0.01;
    int m_freeSizeGbNoNotify = 2;
    int m_minNotifyPeriodMinutes = 10;
    int m_checkIntervalSec = 60;
    QStringList m_ignorePaths;
    QTimer* m_mountChangeThrottleTimer = nullptr;
    bool m_mountChangeThrottled = false;

    // 已通知过的路径及其上次快照时间与数据。
    QHash<QString, NotifyRecord> m_notified;

    // 待提示的低空间挂载队列。
    QVector<MountSnap> m_pendingMounts;
    int m_nextPendingIndex = 0;
};

}  // namespace Kiran
