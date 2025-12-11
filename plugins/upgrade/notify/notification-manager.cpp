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

#include <libnotify/notify.h>

#include <kiran-log/qt5-log-i.h>
#include <qt5-log-i.h>
#include <upgrade-i.h>
#include <QDBusConnection>
#include <QProcess>
#include "ksd_upgrade_proxy.h"
#include "lib/base/def.h"
#include "lib/base/log.h"
#include "notification-manager.h"
namespace Kiran
{
#define PROPERTY_REMINDER_INTERVAL "reminder_interval"
#define PROPERTY_LAST_REMINDER_TIME "latest_reminder_time"
#define TIMEOUT_INTERVAL_MS (24 * 60 * 60 * 1000)
NotificationManager::NotificationManager()
    : m_upgradeProxy(nullptr),
      m_hasUpgrades(false),
      m_scanInProgress(false)
{
    // 初始化libnotify
    if (!notify_init("kcd-upgrade-notify"))
    {
        KLOG_ERROR(upgrade) << "Failed to init libnotify";
    }

    init();
}

NotificationManager::~NotificationManager()
{
    if (m_upgradeProxy)
    {
        delete m_upgradeProxy;
        m_upgradeProxy = nullptr;
    }
    if (notify_is_initted)
    {
        notify_uninit();
    }
}

void NotificationManager::init()
{
    // 初始化升级代理
    m_upgradeProxy = getUpgradeProxy();
    if (!m_upgradeProxy)
    {
        KLOG_ERROR(upgrade) << "Failed to get upgrade proxy";
        return;
    }
    connect(m_upgradeProxy, &KSDUpgradeProxy::ScanCompleted,
            this, &NotificationManager::handleScanResult);
    connect(m_upgradeProxy, &KSDUpgradeProxy::dbusPropertyChanged,
            this, &NotificationManager::handleDBusPropertyChanged);

    //获取最新提醒时间
    m_latestReminderTime = QDateTime::fromString(m_upgradeProxy->latest_reminder_time(), DEFAULT_DATE_TIME_FORMAT);

    // 初始化定时器
    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    connect(m_timer, &QTimer::timeout, this, &NotificationManager::reminder);
    //计算下次提醒时间
    scheduleNextReminder();
}

void NotificationManager::reminder()
{
    QDateTime currentTime = QDateTime::currentDateTime();

    if (m_nextReminderTime.isValid() && m_nextReminderTime > currentTime)
    {
        KLOG_DEBUG(upgrade) << "Timer triggered but not yet time, continuing to wait. "
                            << "Next reminder at " << m_nextReminderTime.toString(DEFAULT_DATE_TIME_FORMAT);
        startTimerForNextReminder(m_nextReminderTime);
        return;
    }

    if (m_scanInProgress)
    {
        KLOG_DEBUG(upgrade) << "Scan in progress, waiting...";
        return;
    }

    // 开始扫描
    m_scanInProgress = true;
    KLOG_DEBUG(upgrade) << "Starting scan for upgradeable packages...";

    auto reply = m_upgradeProxy->Scan();
    reply.waitForFinished();
    if (reply.isError())
    {
        KLOG_ERROR(upgrade) << "Failed to scan for upgradeable packages:" << reply.error().message();
        KLOG_DEBUG(upgrade) << "Scheduling reminder in 24 hour";
        setNextReminder(1);
        return;
    }
}

void NotificationManager::handleScanResult(bool success, const QString &errorMessage)
{
    m_scanInProgress = false;
    m_hasUpgrades = false;

    if (!errorMessage.isEmpty())
    {
        KLOG_ERROR(upgrade) << "Scan failed:" << errorMessage;
        setNextReminder(1);
        return;
    }

    QDBusPendingReply<QString> reply = m_upgradeProxy->GetUpgradePkgsInfo();
    reply.waitForFinished();

    if (reply.isError())
    {
        KLOG_ERROR(upgrade) << "Failed to get upgrade packages info:" << reply.error().message();
        return;
    }

    QString upgradePkgsInfo = reply.value();
    m_hasUpgrades = !upgradePkgsInfo.isEmpty();

    if (m_hasUpgrades)
    {
        sendNotification();
    }
    else
    {
        KLOG_INFO(upgrade) << "System is up to date";
    }

    // 成功：设置下次提醒
    int configInterval = getVaildIntval();
    setNextReminder(configInterval);
}

void NotificationManager::handleDBusPropertyChanged(const QString &name, const QVariant &value)
{
    KLOG_DEBUG(upgrade) << "DBus property changed: " << name << " to " << value.toString();
    RETURN_IF_FALSE(name == PROPERTY_REMINDER_INTERVAL || name == PROPERTY_LAST_REMINDER_TIME);

    if (name == PROPERTY_LAST_REMINDER_TIME)
    {
        m_latestReminderTime = QDateTime::fromString(value.toString(), DEFAULT_DATE_TIME_FORMAT);
    }

    // 清空下次提醒时间，以便基于新的上次提醒时间重新计算
    m_nextReminderTime = QDateTime();
    // 计算下次提醒时间（基于新的上次提醒时间重新计算）
    scheduleNextReminder();
}

KSDUpgradeProxy *NotificationManager::getUpgradeProxy()
{
    auto upgradeProxy = new KSDUpgradeProxy(
        UPGRADE_DBUS_NAME,
        UPGRADE_OBJECT_PATH,
        QDBusConnection::systemBus(),
        this);

    if (!upgradeProxy->isValid())
    {
        KLOG_ERROR(upgrade) << "Failed to create upgrade proxy:" << upgradeProxy->lastError().message();
        return nullptr;
    }

    KLOG_DEBUG(upgrade) << "Upgrade proxy initialized successfully";
    return upgradeProxy;
}

int NotificationManager::getVaildIntval()
{
    auto reminderInterval = m_upgradeProxy->reminder_interval();
    bool isValid = (reminderInterval == REMINDER_INTERVAL_NEVER ||
                    reminderInterval == REMINDER_INTERVAL_WEEKLY ||
                    reminderInterval == REMINDER_INTERVAL_MONTHLY ||
                    reminderInterval == REMINDER_INTERVAL_QUARTERLY);

    if (!isValid)
    {
        KLOG_WARNING(upgrade) << "Invalid reminder interval value: " << reminderInterval
                              << ", resetting to default: " << DEFAULT_REMINDER_INTERVAL;
        reminderInterval = DEFAULT_REMINDER_INTERVAL;
    }

    return reminderInterval;
}

void NotificationManager::startTimerForNextReminder(const QDateTime &nextTime)
{
    QDateTime currentTime = QDateTime::currentDateTime();

    if (nextTime <= currentTime)
    {
        // 已到期，立即触发
        QTimer::singleShot(0, this, &NotificationManager::reminder);
        return;
    }

    // 计算剩余时间并设置定时器
    qint64 msecs = currentTime.msecsTo(nextTime);
    if (msecs > INT_MAX)
    {
        // 超过INT_MAX，分阶段设置
        m_timer->start(INT_MAX);
        KLOG_DEBUG(upgrade) << "Reminder interval exceeds INT_MAX, setting timer to INT_MAX first. "
                            << "Will continue waiting after this timer expires.";
    }
    else
    {
        m_timer->start(static_cast<int>(msecs));
    }
}

void NotificationManager::calculateAndSetNextReminder(int intervalDays, const QDateTime &baseTime)
{
    if (intervalDays == REMINDER_INTERVAL_NEVER)
    {
        m_timer->stop();
        m_nextReminderTime = QDateTime();
        return;
    }

    QDateTime currentTime = QDateTime::currentDateTime();

    // 计算并保存下次提醒时间
    m_nextReminderTime = baseTime.addDays(intervalDays);

    // 如果下次提醒时间已过，立即提醒
    if (m_nextReminderTime <= currentTime)
    {
        QTimer::singleShot(0, this, &NotificationManager::reminder);
        return;
    }

    // 设置定时器
    startTimerForNextReminder(m_nextReminderTime);

    KLOG_DEBUG(upgrade) << "Next reminder in " << intervalDays << " days (at "
                        << m_nextReminderTime.toString(DEFAULT_DATE_TIME_FORMAT) << ")";
}

void NotificationManager::setNextReminder(int intervalDays)
{
    QDateTime currentTime = QDateTime::currentDateTime();

    // 记录本次提醒时间
    m_latestReminderTime = currentTime;
    m_upgradeProxy->setLatest_reminder_time(currentTime.toString(DEFAULT_DATE_TIME_FORMAT));

    // 从当前时间开始计算下次提醒时间
    calculateAndSetNextReminder(intervalDays, currentTime);
}

void NotificationManager::scheduleNextReminder()
{
    // 如果有保存的下次提醒时间，基于它设置定时器
    if (m_nextReminderTime.isValid())
    {
        startTimerForNextReminder(m_nextReminderTime);
        return;
    }

    // 如果没有提醒过，则立即提醒
    if (m_latestReminderTime.isNull())
    {
        QTimer::singleShot(0, this, &NotificationManager::reminder);
        return;
    }

    // 已提醒过但没有保存下次提醒时间，基于上次提醒时间和配置间隔重新计算
    int configInterval = getVaildIntval();
    calculateAndSetNextReminder(configInterval, m_latestReminderTime);
}

void NotificationManager::sendNotification()
{
    if (!notify_is_initted)
    {
        KLOG_WARNING(upgrade) << "libnotify is not initialized";
        return;
    }

    // 避免多次超时时弹出多个消息框
    if (!m_notification.isNull())
    {
        KLOG_DEBUG(upgrade) << "Notification already exists.";
        return;
    }

    auto title = tr("Packages update has been detected.");
    auto body = tr("Please visit the Control Center to upgrade it.");
    m_notification = QSharedPointer<NotifyNotification>(notify_notification_new(title.toUtf8().constData(), body.toUtf8().constData(), NULL),
                                                        g_object_unref);

    notify_notification_set_timeout(m_notification.data(), NOTIFY_EXPIRES_NEVER);

    //点击按钮时启动 kiran-control-panel程序
    typedef void (*NotifyActionCallbackType)(NotifyNotification * notification, char *action, gpointer user_data);
    auto actionCallback = [](NotifyNotification *notification, char *action, gpointer user_data)
    {
        Q_UNUSED(notification);
        Q_UNUSED(action);
        Q_UNUSED(user_data);

        KLOG_DEBUG(upgrade) << "Opening kiran-control-panel from notification action";
        if (!QProcess::startDetached("kiran-control-panel", QStringList()))
        {
            KLOG_ERROR(upgrade) << "Failed to start kiran-control-panel";
        }
    };
    notify_notification_add_action(m_notification.data(),
                                   "open-control-center",
                                   tr("Open Control Center").toUtf8().constData(),
                                   NOTIFY_ACTION_CALLBACK(actionCallback),
                                   this,
                                   NULL);

    // 监听消息关闭信号
    typedef void (*notificationClosedCBType)(NotifyNotification * notification,
                                             gpointer user_data);
    auto callback = [](NotifyNotification *notification, gpointer user_data)
    {
        NotificationManager *manager = static_cast<NotificationManager *>(user_data);
        if (manager && manager->m_notification.data() == notification)
        {
            KLOG_DEBUG(upgrade) << "notification closed";
            manager->m_notification.clear();
        }
    };
    g_signal_connect(m_notification.data(), "closed",
                     G_CALLBACK(static_cast<notificationClosedCBType>(callback)),
                     this);

    // 显示通知
    g_autoptr(GError) error = nullptr;
    if (!notify_notification_show(m_notification.data(), &error))
    {
        KLOG_ERROR(upgrade) << "Failed to show notification:" << (error ? error->message : "unknown error");
        m_notification.clear();
    }
}

}  // namespace Kiran