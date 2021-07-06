/**
 * @Copyright (C) 2020 ~ 2021 KylinSec Co., Ltd. 
 *
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; If not, see <http: //www.gnu.org/licenses/>. 
 */

#include "plugins/power/event/power-event-control.h"

#include "plugins/power/save/power-save.h"

namespace Kiran
{
#define POWER_CRITICAL_ACTION_DELAY 20
PowerEventControl::PowerEventControl(PowerWrapperManager* wrapper_manager,
                                     PowerBacklight* backlight) : wrapper_manager_(wrapper_manager),
                                                                  backlight_(backlight),
                                                                  lid_closed_throttle_(0)
{
    this->backlight_kbd_ = backlight->get_backlight_device(PowerDeviceType::POWER_DEVICE_TYPE_KBD);
    this->kbd_last_nozero_brightness_ = this->backlight_kbd_->get_brightness();
    this->backlight_monitor_ = backlight->get_backlight_device(PowerDeviceType::POWER_DEVICE_TYPE_MONITOR);

    this->upower_client_ = this->wrapper_manager_->get_default_upower();
    this->screensaver_ = this->wrapper_manager_->get_default_screensaver();

    this->power_settings_ = Gio::Settings::create(POWER_SCHEMA_ID);
}

PowerEventControl::~PowerEventControl()
{
}

PowerEventControl* PowerEventControl::instance_ = nullptr;
void PowerEventControl::global_init(PowerWrapperManager* wrapper_manager,
                                    PowerBacklight* backlight)
{
    instance_ = new PowerEventControl(wrapper_manager, backlight);
    instance_->init();
}

void PowerEventControl::init()
{
    event_button_.init();

    event_button_.signal_button_changed().connect(sigc::mem_fun(this, &PowerEventControl::on_button_changed));
    this->backlight_kbd_->signal_brightness_changed().connect(sigc::mem_fun(this, &PowerEventControl::on_kbd_brightness_changed));
    this->upower_client_->signal_device_status_changed().connect(sigc::mem_fun(this, &PowerEventControl::on_device_status_changed));
}

bool PowerEventControl::do_critical_action(PowerAction action)
{
    std::string error;
    if (!PowerSave::get_instance()->do_save(action, error))
    {
        KLOG_WARNING("%s", error.c_str());
    }
    return false;
}

void PowerEventControl::on_button_changed(PowerEvent evnet)
{
    std::string error;
    PowerAction action = PowerAction::POWER_ACTION_NOTHING;

    switch (evnet)
    {
    case POWER_EVENT_PRESSED_POWEROFF:
    {
        action = PowerAction(this->power_settings_->get_enum(POWER_SCHEMA_BUTTON_POWER_ACTION));
        PowerSave::get_instance()->do_save(action, error);
        break;
    }
    case POWER_EVENT_PRESSED_SLEEP:
    case POWER_EVENT_PRESSED_SUSPEND:
    {
        action = PowerAction(this->power_settings_->get_enum(POWER_SCHEMA_BUTTON_SUSPEND_ACTION));
        PowerSave::get_instance()->do_save(action, error);
        break;
    }
    case POWER_EVENT_PRESSED_HIBERNATE:
    {
        action = PowerAction(this->power_settings_->get_enum(POWER_SCHEMA_BUTTON_HIBERNATE_ACTION));
        PowerSave::get_instance()->do_save(action, error);
        break;
    }
    case POWER_EVENT_LID_OPEN:
    {
        PowerSave::get_instance()->do_save(PowerAction::POWER_ACTION_DISPLAY_ON, error);
        if (this->lid_closed_throttle_)
        {
            this->screensaver_->remove_throttle(this->lid_closed_throttle_);
            this->lid_closed_throttle_ = 0;
        }
        break;
    }
    case POWER_EVENT_LID_CLOSED:
    {
        action = PowerAction(this->power_settings_->get_enum(POWER_SCHEMA_LID_CLOSED_ACTION));
        PowerSave::get_instance()->do_save(action, error);
        if (this->lid_closed_throttle_)
        {
            this->screensaver_->remove_throttle(this->lid_closed_throttle_);
        }
        this->lid_closed_throttle_ = this->screensaver_->add_throttle("Laptop lid is closed");
        break;
    }
    case POWER_EVENT_PRESSED_BRIGHT_UP:
        this->backlight_monitor_->brightness_up();
        break;
    case POWER_EVENT_PRESSED_BRIGHT_DOWN:
        this->backlight_monitor_->brightness_down();
        break;
    case POWER_EVENT_PRESSED_KBD_BRIGHT_UP:
        this->backlight_kbd_->brightness_up();
        break;
    case POWER_EVENT_PRESSED_KBD_BRIGHT_DOWN:
        this->backlight_kbd_->brightness_down();
        break;
    case POWER_EVENT_PRESSED_KBD_BRIGHT_TOGGLE:
    {
        if (this->backlight_kbd_->get_brightness() > 0)
        {
            this->backlight_kbd_->set_brightness(0);
        }
        else
        {
            this->backlight_kbd_->set_brightness(this->kbd_last_nozero_brightness_);
        }
        break;
    }
    case POWER_EVENT_PRESSED_LOCK:
        this->screensaver_->lock();
        break;
    // TODO: 显示电池相关的信息，暂不处理该事件
    case POWER_EVENT_PRESSED_BATTERY:
        break;
    default:
        break;
    }

    if (error.length() > 0)
    {
        KLOG_WARNING("%s", error.c_str());
    }
}

void PowerEventControl::on_kbd_brightness_changed(int32_t brightness_value)
{
    if (brightness_value > 0)
    {
        this->kbd_last_nozero_brightness_ = brightness_value;
    }
}

void PowerEventControl::on_device_status_changed(std::shared_ptr<PowerUPowerDevice> device, UPowerDeviceEvent event)
{
    const UPowerDeviceProps& device_props = device->get_props();
    PowerAction action = PowerAction::POWER_ACTION_NOTHING;

    RETURN_IF_FALSE(event == UPowerDeviceEvent::UPOWER_DEVICE_EVENT_CHARGE_ACTION);

    // 如果电池电量过低，但是未使用则忽略
    if (device_props.type == UP_DEVICE_KIND_BATTERY &&
        !this->upower_client_->get_on_battery())
    {
        return;
    }

    switch (device_props.type)
    {
    case UP_DEVICE_KIND_BATTERY:
        action = PowerAction(this->power_settings_->get_enum(POWER_SCHEMA_BATTERY_CRITICAL_ACTION));
        break;
    case UP_DEVICE_KIND_UPS:
        action = PowerAction(this->power_settings_->get_enum(POWER_SCHEMA_UPS_CRITICAL_ACTION));
        break;
        // Ignore other type
    default:
        return;
    }

    // 电量过低执行节能操作，延时执行让用户处理一些紧急事务
    auto timeout = Glib::MainContext::get_default()->signal_timeout();
    timeout.connect_seconds(sigc::bind(sigc::mem_fun(this, &PowerEventControl::do_critical_action), action),
                            POWER_CRITICAL_ACTION_DELAY);
}

}  // namespace Kiran