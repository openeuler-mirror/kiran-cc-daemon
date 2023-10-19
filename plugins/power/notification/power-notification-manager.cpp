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

#include "plugins/power/notification/power-notification-manager.h"

#include <glib/gi18n.h>

#include "plugins/power/power-utils.h"
#include "power-i.h"

namespace Kiran
{
#define POWER_NOTIFY_TIMEOUT_NEVER 0         /* ms */
#define POWER_NOTIFY_TIMEOUT_SHORT 10 * 1000 /* ms */
#define POWER_NOTIFY_TIMEOUT_LONG 30 * 1000  /* ms */

PowerNotificationManager::PowerNotificationManager(PowerWrapperManager *wrapper_manager) : wrapper_manager_(wrapper_manager),
                                                                                           device_notification_(NULL)
{
    this->upower_client_ = wrapper_manager_->get_default_upower();
    this->device_notification_ = notify_notification_new(NULL, NULL, NULL);
    this->power_settings_ = Gio::Settings::create(POWER_SCHEMA_ID);
}

PowerNotificationManager::~PowerNotificationManager()
{
    g_clear_pointer(&this->device_notification_, g_object_unref);
}

PowerNotificationManager *PowerNotificationManager::instance_ = nullptr;
void PowerNotificationManager::global_init(PowerWrapperManager *wrapper_manager)
{
    instance_ = new PowerNotificationManager(wrapper_manager);
    instance_->init();
}

void PowerNotificationManager::init()
{
    this->upower_client_->signal_device_status_changed().connect(sigc::mem_fun(this, &PowerNotificationManager::on_device_status_changed));
}

bool PowerNotificationManager::message_notify(const std::string &title,
                                              const std::string &message,
                                              uint32_t timeout,
                                              const std::string &icon_name,
                                              NotifyUrgency urgency)
{
    GError *error = NULL;

    // 关闭之前的通知
    if (!notify_notification_close(this->device_notification_, &error))
    {
        KLOG_DEBUG_POWER("%s", error->message);
        g_error_free(error);
        error = NULL;
    }

    notify_notification_update(this->device_notification_, title.c_str(), message.c_str(), icon_name.c_str());

    notify_notification_set_timeout(this->device_notification_, timeout);
    notify_notification_set_urgency(this->device_notification_, urgency);
    if (!notify_notification_show(this->device_notification_, &error))
    {
        KLOG_WARNING_POWER("%s", error->message);
        g_error_free(error);
        return false;
    }
    return true;
}

void PowerNotificationManager::on_device_status_changed(std::shared_ptr<PowerUPowerDevice> device, UPowerDeviceEvent event)
{
    switch (event)
    {
    case UPowerDeviceEvent::UPOWER_DEVICE_EVENT_DISCHARGING:
        this->on_device_discharging(device);
        break;
    case UPowerDeviceEvent::UPOWER_DEVICE_EVENT_FULLY_CHARGED:
        this->on_device_fully_charged(device);
        break;
    case UPowerDeviceEvent::UPOWER_DEVICE_EVENT_CHARGE_LOW:
        this->on_device_charge_low(device);
        break;
    case UPowerDeviceEvent::UPOWER_DEVICE_EVENT_CHARGE_CRITICAL:
        this->on_device_charge_critical(device);
        break;
    case UPowerDeviceEvent::UPOWER_DEVICE_EVENT_CHARGE_ACTION:
        this->on_device_charge_action(device);
        break;
    default:
        break;
    }
}

void PowerNotificationManager::on_device_discharging(std::shared_ptr<PowerUPowerDevice> device)
{
    const UPowerDeviceProps &device_props = device->get_props();
    std::string title;
    std::string message;

    auto remaining_time_text = PowerUtils::get_time_translation(device_props.time_to_empty);

    switch (device_props.type)
    {
    case UP_DEVICE_KIND_BATTERY:
    {
        title = _("Battery Discharging");
        message = fmt::format(_("{0} of battery power remaining ({1:.1f}%)"), remaining_time_text, device_props.percentage);
        break;
    }
    case UP_DEVICE_KIND_UPS:
    {
        title = _("UPS Discharging");
        message = fmt::format(_("{0} of UPS backup power remaining ({1:.1f}%)"), remaining_time_text, device_props.percentage);
        break;
    }
    default:
        // Ignore other type
        return;
    }

    this->message_notify(title, message, POWER_NOTIFY_TIMEOUT_LONG, "", NOTIFY_URGENCY_NORMAL);
}

void PowerNotificationManager::on_device_fully_charged(std::shared_ptr<PowerUPowerDevice> device)
{
    const UPowerDeviceProps &device_props = device->get_props();

    if (device_props.type == UP_DEVICE_KIND_BATTERY)
    {
        this->message_notify(_("Battery Charged"), "", POWER_NOTIFY_TIMEOUT_SHORT, "", NOTIFY_URGENCY_LOW);
    }
}

void PowerNotificationManager::on_device_charge_low(std::shared_ptr<PowerUPowerDevice> device)
{
    const UPowerDeviceProps &device_props = device->get_props();

    std::string title;
    std::string message;

    // 如果电池电量过低，但是未使用电池则不提示
    if (device_props.type == UP_DEVICE_KIND_BATTERY &&
        !this->upower_client_->get_on_battery())
    {
        return;
    }

    auto remaining_time_text = PowerUtils::get_time_translation(device_props.time_to_empty);

    switch (device_props.type)
    {
    case UP_DEVICE_KIND_BATTERY:
    {
        title = _("Battery low");
        message = fmt::format(_("Approximately <b>{0}</b> remaining ({1:.1f}%)"),
                              remaining_time_text,
                              device_props.percentage);
        break;
    }
    case UP_DEVICE_KIND_UPS:
    {
        title = _("UPS low");
        message = fmt::format(_("Approximately <b>{0}</b> of remaining UPS backup power ({1:.1f}%)"),
                              remaining_time_text,
                              device_props.percentage);
        break;
    }
    case UP_DEVICE_KIND_MOUSE:
        title = _("Mouse battery low");
        message = fmt::format(_("Wireless mouse is low in power ({0:.1f}%)"), device_props.percentage);
        break;
    case UP_DEVICE_KIND_KEYBOARD:
        title = _("Keyboard battery low");
        message = fmt::format(_("Wireless keyboard is low in power ({0:.1f}%)"), device_props.percentage);
        break;
    case UP_DEVICE_KIND_PDA:
        title = _("PDA battery low");
        message = fmt::format(_("PDA is low in power ({0:.1f}%)"), device_props.percentage);
        break;
    case UP_DEVICE_KIND_PHONE:
        title = _("Cell phone battery low");
        message = fmt::format(_("Cell phone is low in power ({0:.1f}%)"), device_props.percentage);
        break;
    case UP_DEVICE_KIND_MEDIA_PLAYER:
        title = _("Media player battery low");
        message = fmt::format(_("Media player is low in power ({0:1f}%)"), device_props.percentage);
        break;
    case UP_DEVICE_KIND_TABLET:
        title = _("Tablet battery low");
        message = fmt::format(_("Tablet is low in power ({0:1f}%)"), device_props.percentage);
        break;
    case UP_DEVICE_KIND_COMPUTER:
        title = _("Attached computer battery low");
        message = fmt::format(_("Attached computer is low in power ({0:.1f}%)"), device_props.percentage);
        break;
    default:
        // Ignore other type
        return;
    }

    this->message_notify(title, message, POWER_NOTIFY_TIMEOUT_LONG, "", NOTIFY_URGENCY_NORMAL);
}

void PowerNotificationManager::on_device_charge_critical(std::shared_ptr<PowerUPowerDevice> device)
{
    const UPowerDeviceProps &device_props = device->get_props();

    std::string title;
    std::string message;

    // 如果电池电量过低，但是未使用电池则不提示
    if (device_props.type == UP_DEVICE_KIND_BATTERY &&
        !this->upower_client_->get_on_battery())
    {
        return;
    }

    auto remaining_time_text = PowerUtils::get_time_translation(device_props.time_to_empty);

    switch (device_props.type)
    {
    case UP_DEVICE_KIND_BATTERY:
    {
        title = _("Battery critically low");
        message = fmt::format(_("Approximately <b>{0}</b> remaining ({1:.1f}%). Plug in your AC adapter to avoid losing data."),
                              remaining_time_text,
                              device_props.percentage);
        break;
    }
    case UP_DEVICE_KIND_UPS:
        title = _("UPS critically low");
        message = fmt::format(_("Approximately <b>{0}</b> of remaining UPS power ({1:.1f}%). Restore AC power to your computer to avoid losing data."),
                              remaining_time_text,
                              device_props.percentage);
        break;
    case UP_DEVICE_KIND_MOUSE:
        title = _("Mouse battery low");
        message = fmt::format(_("Wireless mouse is very low in power ({0:.1f}%). This device will soon stop functioning if not charged."),
                              device_props.percentage);
        break;
    case UP_DEVICE_KIND_KEYBOARD:
        title = _("Keyboard battery low");
        message = fmt::format(_("Wireless keyboard is very low in power ({0:.1f}%). This device will soon stop functioning if not charged."),
                              device_props.percentage);
        break;
    case UP_DEVICE_KIND_PDA:
        title = _("PDA battery low");
        message = fmt::format(_("PDA is very low in power ({0:.1f}%). This device will soon stop functioning if not charged."),
                              device_props.percentage);
        break;
    case UP_DEVICE_KIND_PHONE:
        title = _("Cell phone battery low");
        message = fmt::format(_("Cell phone is very low in power ({0:.1f}%). This device will soon stop functioning if not charged."),
                              device_props.percentage);
        break;
    case UP_DEVICE_KIND_MEDIA_PLAYER:
        title = _("Cell phone battery low");
        message = fmt::format(_("Media player is very low in power ({0:.1f}%). This device will soon stop functioning if not charged."),
                              device_props.percentage);
        break;
    case UP_DEVICE_KIND_TABLET:
        title = _("Tablet battery low");
        message = fmt::format(_("Tablet is very low in power ({0:.1f}%). This device will soon stop functioning if not charged."),
                              device_props.percentage);
        break;
    case UP_DEVICE_KIND_COMPUTER:
        title = _("Attached computer battery low");
        message = fmt::format(_("Attached computer is very low in power ({0:.1f}%). The device will soon shutdown if not charged."),
                              device_props.percentage);
        break;
    default:
        // Ignore other type
        return;
    }

    this->message_notify(title, message, POWER_NOTIFY_TIMEOUT_NEVER, "", NOTIFY_URGENCY_CRITICAL);
}

void PowerNotificationManager::on_device_charge_action(std::shared_ptr<PowerUPowerDevice> device)
{
    const UPowerDeviceProps &device_props = device->get_props();

    std::string title;
    std::string message;

    // 如果电池电量过低，但是未使用电池则不提示
    if (device_props.type == UP_DEVICE_KIND_BATTERY &&
        !this->upower_client_->get_on_battery())
    {
        return;
    }

    auto remaining_time_text = PowerUtils::get_time_translation(device_props.time_to_empty);

    switch (device_props.type)
    {
    case UP_DEVICE_KIND_BATTERY:
    {
        title = _("Laptop battery critically low");

        auto action = PowerAction(this->power_settings_->get_enum(POWER_SCHEMA_BATTERY_CRITICAL_ACTION));

        switch (action)
        {
        case PowerAction::POWER_ACTION_COMPUTER_SUSPEND:
            message = _(
                "The battery is below the critical level and this computer is about to suspend.<br>"
                "<b>NOTE:</b> A small amount of power is required to keep your computer in a suspended state.");
            break;
        case PowerAction::POWER_ACTION_COMPUTER_HIBERNATE:
            message = _("The battery is below the critical level and this computer is about to hibernate.");
            break;
        case PowerAction::POWER_ACTION_COMPUTER_SHUTDOWN:
            message = _("The battery is below the critical level and this computer is about to shutdown.");
            break;
        default:
            message = _("The battery is below the critical level and this computer will <b>power-off</b> when the battery becomes completely empty.");
            break;
        }
        break;
    }
    case UP_DEVICE_KIND_UPS:
    {
        title = _("UPS critically low");

        auto action = PowerAction(this->power_settings_->get_enum(POWER_SCHEMA_UPS_CRITICAL_ACTION));

        switch (action)
        {
        case PowerAction::POWER_ACTION_COMPUTER_SUSPEND:
            message = _(
                "The UPS is below the critical level and this computer is about to suspend.<br>"
                "<b>NOTE:</b> A small amount of power is required to keep your computer in a suspended state.");
            break;
        case PowerAction::POWER_ACTION_COMPUTER_HIBERNATE:
            message = _("The UPS is below the critical level and this computer is about to hibernate.");
            break;
        case PowerAction::POWER_ACTION_COMPUTER_SHUTDOWN:
            message = _("The UPS is below the critical level and this computer is about to shutdown.");
            break;
        default:
            message = _("The UPS is below the critical level and this computer will <b>power-off</b> when the UPS becomes completely empty.");
            break;
        }
        break;
    }
    default:
        // Ignore other type
        return;
    }

    this->message_notify(title, message, POWER_NOTIFY_TIMEOUT_NEVER, "", NOTIFY_URGENCY_CRITICAL);
}
}  // namespace Kiran