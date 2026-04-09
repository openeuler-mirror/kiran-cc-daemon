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

#include <libnotify/notify.h>

#include "low-disk-space-notifier.h"

#include <QLocale>
#include <qt5-log-i.h>

#include <glib.h>

namespace Kiran
{

namespace
{
constexpr char kNotifyAppName[] = "kiran-cc-daemon";
constexpr char kActionAnalyze[] = "analyze";
constexpr char kActionIgnore[] = "ignore";
}  // namespace

LowDiskSpaceNotifier::LowDiskSpaceNotifier(QObject* parent) : QObject(parent) {}

bool LowDiskSpaceNotifier::ensureNotifyInit()
{
    if (notify_is_initted())
        return true;
    if (!notify_init(kNotifyAppName))
    {
        KLOG_WARNING() << "disk-space: libnotify init failed";
        return false;
    }
    return true;
}

bool LowDiskSpaceNotifier::hasActiveNotification() const
{
    return !m_notification.isNull();
}

void LowDiskSpaceNotifier::onNotificationClosed(NotifyNotification* notification, void* userData)
{
    auto* self = static_cast<LowDiskSpaceNotifier*>(userData);
    if (!self || self->m_notification.data() != notification)
        return;

    const bool handledByAction = self->m_responseSent;
    self->m_notification.clear();
    if (handledByAction)
        return;

    QMetaObject::invokeMethod(self, "emitResponse", Qt::QueuedConnection, Q_ARG(bool, false));
}

void LowDiskSpaceNotifier::onNotificationAction(NotifyNotification* notification, char* action, void* userData)
{
    Q_UNUSED(notification);
    auto* self = static_cast<LowDiskSpaceNotifier*>(userData);
    if (!self || !action)
    {
        return;
    }

    self->m_responseSent = true;
    bool launch = false;
    if (qstrcmp(action, kActionAnalyze) == 0)
    {
        launch = true;
    }
    else if (qstrcmp(action, kActionIgnore) != 0)
    {
        KLOG_WARNING() << "disk-space: unknown notification action" << action;
    }

    QMetaObject::invokeMethod(self, "emitResponse", Qt::QueuedConnection, Q_ARG(bool, launch));
}

void LowDiskSpaceNotifier::emitResponse(bool launchedAnalyzer)
{
    emit response(launchedAnalyzer, m_mountPath);
}

bool LowDiskSpaceNotifier::show(bool showAnalyze,
                                qint64 freeBytes,
                                const QString& volumeName,
                                const QString& mountPath)
{
    if (!m_notification.isNull())
        m_notification.clear();

    m_responseSent = false;
    m_mountPath = mountPath;

    if (!ensureNotifyInit())
        return false;

    const QString title = tr("Low Disk Space");
    const QString freeStr = QLocale().formattedDataSize(freeBytes, 2, QLocale::DataSizeTraditionalFormat);
    const QString primary = tr("The volume \"%1\" has only %2 disk space remaining.").arg(volumeName, freeStr);

    const QString secondary =
        tr("You can free up disk space by removing unused programs or files, "
           "or by moving files to another disk or partition.");
    const QString body = QStringLiteral("%1\n\n%2").arg(primary, secondary);

    m_notification = QSharedPointer<NotifyNotification>(
        notify_notification_new(title.toUtf8().constData(), body.toUtf8().constData(), nullptr), g_object_unref);

    notify_notification_set_timeout(m_notification.data(), NOTIFY_EXPIRES_NEVER);

    if (showAnalyze)
    {
        notify_notification_add_action(m_notification.data(), kActionAnalyze, tr("Analyze").toUtf8().constData(),
                                       NOTIFY_ACTION_CALLBACK(LowDiskSpaceNotifier::onNotificationAction), this, nullptr);
    }
    notify_notification_add_action(m_notification.data(), kActionIgnore, tr("Ignore").toUtf8().constData(),
                                   NOTIFY_ACTION_CALLBACK(LowDiskSpaceNotifier::onNotificationAction), this, nullptr);

    g_signal_connect(m_notification.data(), "closed", G_CALLBACK(LowDiskSpaceNotifier::onNotificationClosed), this);

    g_autoptr(GError) error = nullptr;
    if (!notify_notification_show(m_notification.data(), &error))
    {
        KLOG_ERROR() << "disk-space: failed to show notification:" << (error ? error->message : "unknown error");
        m_notification.clear();
        return false;
    }
    return true;
}

}  // namespace Kiran
