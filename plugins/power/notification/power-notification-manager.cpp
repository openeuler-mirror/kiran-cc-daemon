/**
 * Copyright (c) 2020 ~ 2021 KylinSec Co., Ltd.
 * kiran-cc-daemon is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
 */

#include <libnotify/notification.h>
#include <libnotify/notify.h>
#include <QGSettings>
#include "../power-utils.h"
#include "../wrapper/power-upower-device.h"
#include "../wrapper/power-upower.h"
#include "../wrapper/power-wrapper-manager.h"
#include "lib/base/base.h"
#include "power-i.h"
#include "power-notification-manager.h"

namespace Kiran
{
#define POWER_NOTIFY_TIMEOUT_NEVER 0         /* ms */
#define POWER_NOTIFY_TIMEOUT_SHORT 10 * 1000 /* ms */
#define POWER_NOTIFY_TIMEOUT_LONG 30 * 1000  /* ms */

PowerNotificationManager::PowerNotificationManager(PowerWrapperManager *wrapperManager) : m_wrapperManager(wrapperManager),
                                                                                          m_deviceNotification(NULL)
{
    m_upowerClient = m_wrapperManager->getDefaultUpower();
    notify_init(tr("Control Center").toUtf8().data());
    m_deviceNotification = notify_notification_new(NULL, NULL, NULL);
    m_powerSettings = new QGSettings(POWER_SCHEMA_ID, "", this);
}

PowerNotificationManager::~PowerNotificationManager()
{
    g_clear_pointer(&m_deviceNotification, g_object_unref);
}

PowerNotificationManager *PowerNotificationManager::m_instance = nullptr;
void PowerNotificationManager::globalInit(PowerWrapperManager *wrapper_manager)
{
    m_instance = new PowerNotificationManager(wrapper_manager);
    m_instance->init();
}

void PowerNotificationManager::init()
{
    connect(m_upowerClient.get(), &PowerUPower::deviceStatusChanged, this, &PowerNotificationManager::processDeviceStatusChanged);
}

bool PowerNotificationManager::messageNotify(const QString &title,
                                             const QString &message,
                                             uint32_t timeout,
                                             const QString &iconName,
                                             int urgency)
{
    GError *error = NULL;

    // 关闭之前的通知
    if (!notify_notification_close(m_deviceNotification, &error))
    {
        KLOG_DEBUG(power) << error->message;
        g_error_free(error);
        error = NULL;
    }

    notify_notification_update(m_deviceNotification,
                               title.toUtf8().data(),
                               message.toUtf8().data(),
                               iconName.toUtf8().data());

    notify_notification_set_timeout(m_deviceNotification, timeout);
    notify_notification_set_urgency(m_deviceNotification, NotifyUrgency(urgency));
    if (!notify_notification_show(m_deviceNotification, &error))
    {
        KLOG_WARNING(power) << error->message;
        g_error_free(error);
        return false;
    }
    return true;
}

void PowerNotificationManager::processDeviceStatusChanged(QSharedPointer<PowerUPowerDevice> device, int event)
{
    switch (event)
    {
    case UPowerDeviceEvent::UPOWER_DEVICE_EVENT_DISCHARGING:
        processDeviceDischarging(device);
        break;
    case UPowerDeviceEvent::UPOWER_DEVICE_EVENT_FULLY_CHARGED:
        processDeviceFullyCharged(device);
        break;
    case UPowerDeviceEvent::UPOWER_DEVICE_EVENT_CHARGE_LOW:
        processDeviceChargeLow(device);
        break;
    case UPowerDeviceEvent::UPOWER_DEVICE_EVENT_CHARGE_CRITICAL:
        processDeviceChargeCritical(device);
        break;
    case UPowerDeviceEvent::UPOWER_DEVICE_EVENT_CHARGE_ACTION:
        processDeviceChargeAction(device);
        break;
    default:
        break;
    }
}

void PowerNotificationManager::processDeviceDischarging(QSharedPointer<PowerUPowerDevice> device)
{
    const UPowerDeviceProps &deviceProps = device->getProps();
    QString title;
    QString message;
    QString remainingTimeText;

    /**
    * 笔记本电池FullCharged状态下拔掉电源，TimeToEmpty更新没那么及时(D2000环境大概10s)，导致拔电通知错误的剩余时间(#93013)
    × 如果TimeToEmtpy为一个错误的值，或无法获取到剩余时长，则不通知。
    */
    if (deviceProps.timeToEmpty > 0)
    {
        remainingTimeText = PowerUtils::getTimeTranslation(deviceProps.timeToEmpty);
    }

    switch (deviceProps.type)
    {
    case UP_DEVICE_KIND_BATTERY:
    {
        title = tr("Battery Discharging");
        if (!remainingTimeText.isEmpty())
        {
            message = QString(tr("%1 of battery power remaining (%2%)"))
                          .arg(remainingTimeText)
                          .arg(deviceProps.percentage, 0, 'f', 1);
        }
        break;
    }
    case UP_DEVICE_KIND_UPS:
    {
        title = tr("UPS Discharging");
        if (!remainingTimeText.isEmpty())
        {
            message = QString(tr("%1 of UPS backup power remaining (%2%)"))
                          .arg(remainingTimeText)
                          .arg(deviceProps.percentage, 0, 'f', 1);
        }
        break;
    }
    default:
        // Ignore other type
        return;
    }

    messageNotify(title, message, POWER_NOTIFY_TIMEOUT_LONG, "", NOTIFY_URGENCY_NORMAL);
}

void PowerNotificationManager::processDeviceFullyCharged(QSharedPointer<PowerUPowerDevice> device)
{
    const UPowerDeviceProps &device_props = device->getProps();

    if (device_props.type == UP_DEVICE_KIND_BATTERY)
    {
        messageNotify(tr("Battery Charged"), "", POWER_NOTIFY_TIMEOUT_SHORT, "", NOTIFY_URGENCY_LOW);
    }
}

void PowerNotificationManager::processDeviceChargeLow(QSharedPointer<PowerUPowerDevice> device)
{
    const UPowerDeviceProps &deviceProps = device->getProps();

    QString title;
    QString message;

    // 如果电池电量过低，但是未使用电池则不提示
    if (deviceProps.type == UP_DEVICE_KIND_BATTERY &&
        !m_upowerClient->getOnBattery())
    {
        return;
    }

    auto remainingTimeText = PowerUtils::getTimeTranslation(deviceProps.timeToEmpty);

    switch (deviceProps.type)
    {
    case UP_DEVICE_KIND_BATTERY:
    {
        title = tr("Battery low");
        message = QString(tr("Approximately <b>%1</b> remaining (%2%)"))
                      .arg(remainingTimeText)
                      .arg(deviceProps.percentage, 0, 'f', 1);
        break;
    }
    case UP_DEVICE_KIND_UPS:
    {
        title = tr("UPS low");
        message = QString(tr("Approximately <b>%1</b> of remaining UPS backup power (%2%)"))
                      .arg(remainingTimeText)
                      .arg(deviceProps.percentage, 0, 'f', 1);
        break;
    }
    case UP_DEVICE_KIND_MOUSE:
        title = tr("Mouse battery low");
        message = QString(tr("Wireless mouse is low in power (%1%)")).arg(deviceProps.percentage, 0, 'f', 1);
        break;
    case UP_DEVICE_KIND_KEYBOARD:
        title = tr("Keyboard battery low");
        message = QString(tr("Wireless keyboard is low in power (%1%)")).arg(deviceProps.percentage, 0, 'f', 1);
        break;
    case UP_DEVICE_KIND_PDA:
        title = tr("PDA battery low");
        message = QString(tr("PDA is low in power (%1%)")).arg(deviceProps.percentage, 0, 'f', 1);
        break;
    case UP_DEVICE_KIND_PHONE:
        title = tr("Cell phone battery low");
        message = QString(tr("Cell phone is low in power (%1%)")).arg(deviceProps.percentage, 0, 'f', 1);
        break;
    case UP_DEVICE_KIND_MEDIA_PLAYER:
        title = tr("Media player battery low");
        message = QString(tr("Media player is low in power (%1%)")).arg(deviceProps.percentage, 0, 'f', 1);
        break;
    case UP_DEVICE_KIND_TABLET:
        title = tr("Tablet battery low");
        message = QString(tr("Tablet is low in power (%1%)")).arg(deviceProps.percentage, 0, 'f', 1);
        break;
    case UP_DEVICE_KIND_COMPUTER:
        title = tr("Attached computer battery low");
        message = QString(tr("Attached computer is low in power (%1%)")).arg(deviceProps.percentage, 0, 'f', 1);
        break;
    default:
        // Ignore other type
        return;
    }

    messageNotify(title, message, POWER_NOTIFY_TIMEOUT_LONG, "", NOTIFY_URGENCY_NORMAL);
}

void PowerNotificationManager::processDeviceChargeCritical(QSharedPointer<PowerUPowerDevice> device)
{
    const UPowerDeviceProps &deviceProps = device->getProps();

    QString title;
    QString message;

    // 如果电池电量过低，但是未使用电池则不提示
    if (deviceProps.type == UP_DEVICE_KIND_BATTERY &&
        !m_upowerClient->getOnBattery())
    {
        return;
    }

    auto remainingTimeText = PowerUtils::getTimeTranslation(deviceProps.timeToEmpty);

    switch (deviceProps.type)
    {
    case UP_DEVICE_KIND_BATTERY:
    {
        title = tr("Battery critically low");
        message = QString(tr("Approximately <b>%1</b> remaining (%2%). Plug in your AC adapter to avoid losing data."))
                      .arg(remainingTimeText)
                      .arg(deviceProps.percentage, 0, 'f', 1);
        break;
    }
    case UP_DEVICE_KIND_UPS:
        title = tr("UPS critically low");
        message = QString(tr("Approximately <b>%1</b> of remaining UPS power (%2%). Restore AC power to your computer to avoid losing data."))
                      .arg(remainingTimeText)
                      .arg(deviceProps.percentage, 0, 'f', 1);
        break;
    case UP_DEVICE_KIND_MOUSE:
        title = tr("Mouse battery low");
        message = QString(tr("Wireless mouse is very low in power (%1%). This device will soon stop functioning if not charged."))
                      .arg(deviceProps.percentage, 0, 'f', 1);
        break;
    case UP_DEVICE_KIND_KEYBOARD:
        title = tr("Keyboard battery low");
        message = QString(tr("Wireless keyboard is very low in power (%1%). This device will soon stop functioning if not charged."))
                      .arg(deviceProps.percentage, 0, 'f', 1);
        break;
    case UP_DEVICE_KIND_PDA:
        title = tr("PDA battery low");
        message = QString(tr("PDA is very low in power (%1%). This device will soon stop functioning if not charged."))
                      .arg(deviceProps.percentage, 0, 'f', 1);
        break;
    case UP_DEVICE_KIND_PHONE:
        title = tr("Cell phone battery low");
        message = QString(tr("Cell phone is very low in power (%1%). This device will soon stop functioning if not charged."))
                      .arg(deviceProps.percentage, 0, 'f', 1);
        break;
    case UP_DEVICE_KIND_MEDIA_PLAYER:
        title = tr("Cell phone battery low");
        message = QString(tr("Media player is very low in power (%1%). This device will soon stop functioning if not charged."))
                      .arg(deviceProps.percentage, 0, 'f', 1);
        break;
    case UP_DEVICE_KIND_TABLET:
        title = tr("Tablet battery low");
        message = QString(tr("Tablet is very low in power (%1%). This device will soon stop functioning if not charged."))
                      .arg(deviceProps.percentage, 0, 'f', 1);
        break;
    case UP_DEVICE_KIND_COMPUTER:
        title = tr("Attached computer battery low");
        message = QString(tr("Attached computer is very low in power (%1%). The device will soon shutdown if not charged."))
                      .arg(deviceProps.percentage, 0, 'f', 1);
        break;
    default:
        // Ignore other type
        return;
    }

    messageNotify(title, message, POWER_NOTIFY_TIMEOUT_NEVER, "", NOTIFY_URGENCY_CRITICAL);
}

void PowerNotificationManager::processDeviceChargeAction(QSharedPointer<PowerUPowerDevice> device)
{
    const UPowerDeviceProps &device_props = device->getProps();

    QString title;
    QString message;

    // 如果电池电量过低，但是未使用电池则不提示
    if (device_props.type == UP_DEVICE_KIND_BATTERY &&
        !m_upowerClient->getOnBattery())
    {
        return;
    }

    auto remaining_time_text = PowerUtils::getTimeTranslation(device_props.timeToEmpty);

    switch (device_props.type)
    {
    case UP_DEVICE_KIND_BATTERY:
    {
        title = tr("Laptop battery critically low");

        auto action = PowerAction(m_powerSettings->get(POWER_SCHEMA_BATTERY_CRITICAL_ACTION).toInt());

        switch (action)
        {
        case PowerAction::POWER_ACTION_COMPUTER_SUSPEND:
            message = tr(
                "The battery is below the critical level and this computer is about to suspend.<br>"
                "<b>NOTE:</b> A small amount of power is required to keep your computer in a suspended state.");
            break;
        case PowerAction::POWER_ACTION_COMPUTER_HIBERNATE:
            message = tr("The battery is below the critical level and this computer is about to hibernate.");
            break;
        case PowerAction::POWER_ACTION_COMPUTER_SHUTDOWN:
            message = tr("The battery is below the critical level and this computer is about to shutdown.");
            break;
        default:
            message = tr("The battery is below the critical level and this computer will <b>power-off</b> when the battery becomes completely empty.");
            break;
        }
        break;
    }
    case UP_DEVICE_KIND_UPS:
    {
        title = tr("UPS critically low");

        auto action = PowerAction(m_powerSettings->get(POWER_SCHEMA_UPS_CRITICAL_ACTION).toInt());

        switch (action)
        {
        case PowerAction::POWER_ACTION_COMPUTER_SUSPEND:
            message = tr(
                "The UPS is below the critical level and this computer is about to suspend.<br>"
                "<b>NOTE:</b> A small amount of power is required to keep your computer in a suspended state.");
            break;
        case PowerAction::POWER_ACTION_COMPUTER_HIBERNATE:
            message = tr("The UPS is below the critical level and this computer is about to hibernate.");
            break;
        case PowerAction::POWER_ACTION_COMPUTER_SHUTDOWN:
            message = tr("The UPS is below the critical level and this computer is about to shutdown.");
            break;
        default:
            message = tr("The UPS is below the critical level and this computer will <b>power-off</b> when the UPS becomes completely empty.");
            break;
        }
        break;
    }
    default:
        // Ignore other type
        return;
    }

    messageNotify(title, message, POWER_NOTIFY_TIMEOUT_NEVER, "", NOTIFY_URGENCY_CRITICAL);
}
}  // namespace Kiran