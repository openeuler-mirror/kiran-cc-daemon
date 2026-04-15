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

#include "installer.h"
#include <glib-object.h>
#include <glib.h>
#include "dnf/dnf-wrapper.h"

#include <qt5-log-i.h>
#include <QFuture>
#include <QFutureWatcher>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QtConcurrent>

#include "lib/base/log.h"

namespace Kiran
{
Installer::Installer(QObject *parent) : QObject(parent), m_currentPercentage(0)
{
    m_dnfWrapper = DnfWrapper::instance();

    connect(&m_installerFutureWatcher, &QFutureWatcher<ResultDetail>::finished, this, [this]()
            { emit installCompleted(m_installerFutureWatcher.result().success, m_installerFutureWatcher.result().errorMessage); });

    // 连接 DnfWrapper 的升级进度信号，转发给外部
    connect(m_dnfWrapper, &DnfWrapper::installActionChanged, this, &Installer::handleInstallAction, Qt::QueuedConnection);
    connect(m_dnfWrapper, &DnfWrapper::installPercentageChanged,
            this, &Installer::handleInstallProgress, Qt::QueuedConnection);
}

Installer::~Installer()
{
    if (m_installerFutureWatcher.isRunning())
    {
        m_installerFutureWatcher.waitForFinished();
    }
}

CCErrorCode Installer::install(const QStringList &packageIDs)
{
    if (m_installerFutureWatcher.isRunning())
    {
        KLOG_WARNING(upgrade) << "Installer is already in progress";
        return CCErrorCode::ERROR_UPGRADE_INSTALL_NOT_COMPLETED;
    }

    if (packageIDs.isEmpty())
    {
        KLOG_ERROR(upgrade) << "Package IDs list is empty";
        return CCErrorCode::ERROR_UPGRADE_PACKAGE_IDS_EMPTY;
    }

    //清空安装进度信息
    m_currentPercentage = 0;
    m_installAction.clear();

    m_installerFutureWatcher.setFuture(QtConcurrent::run(this, &Installer::doInstall, packageIDs));
    return CCErrorCode::SUCCESS;
}

Installer::ResultDetail Installer::doInstall(const QStringList &packageIDs)
{
    KLOG_DEBUG(upgrade) << "Starting to upgrade" << packageIDs.size() << "packages";
    QString errorMessage;

    // 执行升级
    bool success = m_dnfWrapper->installPackagesByIds(packageIDs, errorMessage);
    if (!success)
    {
        KLOG_WARNING(upgrade) << "Failed to upgrade packages. " << errorMessage;
        return ResultDetail{false, errorMessage};
    }
    return ResultDetail{true, QString()};
}

QString Installer::getInstallLog()
{
    QJsonObject logObject;
    logObject["percentage"] = static_cast<int>(m_currentPercentage);
    logObject["action"] = m_installAction.trimmed();

    QJsonDocument doc(logObject);
    return doc.toJson(QJsonDocument::Compact);
}
void Installer::handleInstallProgress(uint percentage)
{
    m_currentPercentage = percentage;
    emit installProgressChanged(percentage);
}

void Installer::handleInstallAction(const QString &action, const QString &actionHint)
{
    m_installAction += action + (actionHint.isEmpty() ? "" : ":" + actionHint) + "\n";
    emit installActionChanged(action, actionHint);
}

}  // namespace Kiran