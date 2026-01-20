/**
 * Copyright (c) 2020 ~ 2025 KylinSec Co., Ltd.
 * kiran-cc-daemon is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     yuanxing <yuanxing@kylinsec.com.cn>
 */

#include <glib-object.h>
#include <glib.h>
#include <libdnf/libdnf.h>
#include "dnf/dnf-wrapper.h"

#include <qt5-log-i.h>
#include <QJsonArray>
#include <QMutexLocker>
#include <QtConcurrent>
#include "scanner.h"

#include "lib/base/log.h"

namespace Kiran
{
Scanner::Scanner(QObject *parent) : QObject(parent)
{
    m_dnfWrapper = DnfWrapper::instance();
    connect(&m_scanFutureWatcher, &QFutureWatcher<ResultDetail>::finished, this, [this]()
            { emit scanCompleted(m_scanFutureWatcher.result().success, m_scanFutureWatcher.result().errorMessage); });
}

Scanner::~Scanner()
{
    // 等待异步任务完成
    if (m_scanFutureWatcher.isRunning())
    {
        m_scanFutureWatcher.waitForFinished();
    }
}

CCErrorCode Scanner::scan()
{
    if (m_scanFutureWatcher.isRunning())
    {
        KLOG_WARNING(upgrade) << "Scan is already in progress";
        return CCErrorCode::ERROR_UPGRADE_SCAN_NOT_COMPLETED;
    }

    m_scanFutureWatcher.setFuture(QtConcurrent::run(this, &Scanner::doScan));
    return CCErrorCode::SUCCESS;
}

Scanner::ResultDetail Scanner::doScan()
{
    KLOG_DEBUG(upgrade) << "Starting scan for upgradeable packages...";

    // 清理上次扫描的包
    clearUpgradePkgs();

    // 区分失败与无更新包两种情况
    QString errorMessage;
    auto packages = m_dnfWrapper->getUpgradesPackages(errorMessage);
    if (!errorMessage.isEmpty())
    {
        KLOG_WARNING(upgrade) << "Failed to get upgrades packages. " << errorMessage;
        return ResultDetail{false, errorMessage};
    }
    // 无更新包,正常情况
    if (packages.isEmpty())
    {
        KLOG_DEBUG(upgrade) << "No upgradeable packages found. " << errorMessage;
        return ResultDetail{true, QString()};
    }

    KLOG_DEBUG(upgrade) << "Scan completed, found" << packages.size() << "packages";

    //保存可更新包信息
    for (auto pkg : packages)
    {
        if (pkg.isNull())
        {
            KLOG_WARNING(upgrade) << "Null package pointer found, skipping";
            continue;
        }

        // 获取 package_id 需要访问 sack，确保在获取前包对象有效
        const gchar *pkgIDStr = dnf_package_get_package_id(pkg.data());
        if (!pkgIDStr)
        {
            KLOG_WARNING(upgrade) << "Failed to get package_id, skipping package";
            continue;
        }

        QString pkgID = QString::fromUtf8(pkgIDStr);
        {
            QMutexLocker locker(&m_upgradePkgsMutex);
            m_upgradePkgs[pkgID].swap(pkg);
        }
        KLOG_DEBUG(upgrade) << "Package" << pkgID << "added to upgrade packages";
    }
    return ResultDetail{true, QString()};
}

QString Scanner::getUpgradePkgsJson()
{
    QList<QSharedPointer<::DnfPackage>> packages;
    {
        QMutexLocker locker(&m_upgradePkgsMutex);
        packages = m_upgradePkgs.values();
    }

    if (packages.isEmpty())
    {
        KLOG_WARNING(upgrade) << "No upgradeable packages found.";
        return QString();
    }

    QJsonArray jsonArray;
    for (auto pkg : packages)
    {
        if (pkg.isNull())
        {
            continue;
        }
        QString errorMessage;
        auto pgkName = QString(dnf_package_get_name(pkg.data()));
        auto currentVersion = m_dnfWrapper->getInstalledPackageVersion(pgkName, errorMessage);
        if (!errorMessage.isEmpty())
        {
            KLOG_WARNING(upgrade) << "Failed to get installed package version. " << errorMessage;
        }
        QJsonObject obj;
        obj["id"] = QString::fromUtf8(dnf_package_get_package_id(pkg.data()));
        obj["name"] = pgkName;
        obj["current_version"] = currentVersion;
        obj["latest_version"] = QString(dnf_package_get_evr(pkg.data()));
        auto downloadSize = static_cast<quint64>(dnf_package_get_downloadsize(pkg.data()));
        obj["size"] = formatSize(downloadSize);
        // 获取包的 advisory 信息
        obj["advisory_info"] = getPackageAdvisoryInfo(pkg);

        jsonArray.append(obj);
    }
    QJsonDocument doc(jsonArray);
    return doc.toJson(QJsonDocument::Indented);
}

QMap<QString, QSharedPointer<::DnfPackage>> Scanner::getUpgradePkgs()
{
    {
        QMutexLocker locker(&m_upgradePkgsMutex);
        return m_upgradePkgs;
    }
}

QString Scanner::formatSize(quint64 size)
{
    if (size < 1024)
    {
        return QString::number(size) + "B";
    }
    if (size < 1024 * 1024)
    {
        double kb = static_cast<double>(size) / 1024.0;
        return QString::number(kb, 'f', 1) + "KB";
    }
    if (size < 1024 * 1024 * 1024)
    {
        double mb = static_cast<double>(size) / (1024.0 * 1024.0);
        return QString::number(mb, 'f', 1) + "MB";
    }
    double gb = static_cast<double>(size) / (1024.0 * 1024.0 * 1024.0);
    return QString::number(gb, 'f', 1) + "GB";
}

QJsonObject Scanner::getPackageAdvisoryInfo(QSharedPointer<::DnfPackage> pkg)
{
    QJsonObject advisoryInfo;
    if (pkg.isNull())
    {
        return advisoryInfo;
    }

    // 获取包的 advisories
    GPtrArray *advisoryList = dnf_package_get_advisories(pkg.data(), HY_EQ);
    QStringList advisories;
    AdvisoryKindFlags advisoryKindFlags = ADVISORY_KIND_NONE;

    if (advisoryList && advisoryList->len > 0)
    {
        for (uint i = 0; i < advisoryList->len; i++)
        {
            ::DnfAdvisory *advisory = static_cast<::DnfAdvisory *>(g_ptr_array_index(advisoryList, i));
            if (!advisory)
            {
                continue;
            }

            const char *advisoryId = dnf_advisory_get_id(advisory);
            const char *advisoryTitle = dnf_advisory_get_title(advisory);
            DnfAdvisoryKind advisoryKind = dnf_advisory_get_kind(advisory);

            AdvisoryKindFlag flag = advisoryKindToFlag(advisoryKind);
            advisoryKindFlags |= flag;

            //保存advisory信息中的id、title、kind
            if (advisoryId)
            {
                QString advisoryInfoStr = QString::fromUtf8(advisoryId);
                if (advisoryTitle)
                {
                    advisoryInfoStr += " - " + QString::fromUtf8(advisoryTitle);
                }
                advisoryInfoStr += " [ kind: " + QString::number(flag) + "]";
                advisories.append(advisoryInfoStr);
            }
        }
    }
    advisoryKindFlags = advisoryKindFlags == ADVISORY_KIND_NONE ? ADVISORY_KIND_UNKNOWN : advisoryKindFlags;

    advisoryInfo["advisory_kind"] = static_cast<int>(advisoryKindFlags);

    // 设置 advisories 列表
    if (!advisories.isEmpty())
    {
        advisoryInfo["advisories"] = QJsonArray::fromStringList(advisories);
    }
    else
    {
        advisoryInfo["advisories"] = QJsonArray();
    }

    if (advisoryList)
    {
        g_ptr_array_free(advisoryList, TRUE);
    }

    return advisoryInfo;
}

void Scanner::clearUpgradePkgs()
{
    {
        QMutexLocker locker(&m_upgradePkgsMutex);
        m_upgradePkgs.clear();
    }
}

AdvisoryKindFlag Scanner::advisoryKindToFlag(int kind)
{
    switch (static_cast<DnfAdvisoryKind>(kind))
    {
    case DNF_ADVISORY_KIND_UNKNOWN:
        return ADVISORY_KIND_UNKNOWN;
    case DNF_ADVISORY_KIND_SECURITY:
        return ADVISORY_KIND_SECURITY;
    case DNF_ADVISORY_KIND_BUGFIX:
        return ADVISORY_KIND_BUGFIX;
    case DNF_ADVISORY_KIND_ENHANCEMENT:
        return ADVISORY_KIND_ENHANCEMENT;
    case DNF_ADVISORY_KIND_NEWPACKAGE:
        return ADVISORY_KIND_NEWPACKAGE;
    default:
        return ADVISORY_KIND_UNKNOWN;
    }
}
}  // namespace Kiran