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

#include "plugins/power/event/power-event-control.h"

#include "plugins/power/save/power-save.h"

namespace Kiran
{
#define POWER_CRITICAL_ACTION_DELAY 20
PowerEventControl::PowerEventControl(PowerWrapperManager* wrapper_manager,
                                     PowerBacklight* backlight) : wrapper_manager_(wrapper_manager),
                                                                  backlight_(backlight),
                                                                  lid_closed_throttle_(0),
                                                                  display_dimmed_set_(false)
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

void PowerEventControl::charging_event()
{
    // 未设置过显示器变暗操作，则不要取做恢复操作，因为可能恢复的是其他场景设置的变暗操作。
    RETURN_IF_FALSE(this->display_dimmed_set_);
    PowerSave::get_instance()->do_display_restore_dimmed();
    this->display_dimmed_set_ = false;

    PowerSave::get_instance()->do_cpu_restore_saver();
}

void PowerEventControl::discharging_event(std::shared_ptr<PowerUPowerDevice> device)
{
    const UPowerDeviceProps& device_props = device->get_props();

    // 如果拔掉了电源线，则需要重新判断当前电量情况，然后进行节能操作。
    switch (device_props.warning_level)
    {
    case UpDeviceLevel::UP_DEVICE_LEVEL_LOW:
        this->charge_low_event(device);
        break;
    default:
        break;
    }
}

void PowerEventControl::charge_low_event(std::shared_ptr<PowerUPowerDevice> device)
{
    const UPowerDeviceProps& device_props = device->get_props();

    // 如果电池电量过低，但是未使用则忽略
    if (device_props.type == UP_DEVICE_KIND_BATTERY &&
        !this->upower_client_->get_on_battery())
    {
        return;
    }

    // 执行电量过低时显示器变暗
    auto charge_low_dimmed_enabled = this->power_settings_->get_boolean(POWER_SCHEMA_ENABLE_CHARGE_LOW_DIMMED);
    // 这里必须要判断当前是否处于变暗状态。如果当前已经处于变暗状态，调用do_display_dimmed函数会导致display_dimmed_set_置为false。
    if (charge_low_dimmed_enabled && !PowerSave::get_instance()->is_display_dimmed())
    {
        this->display_dimmed_set_ = PowerSave::get_instance()->do_display_dimmed();
    }

    // 执行电量过低时计算机进入节能模式
    auto charge_low_saver_enabled = this->power_settings_->get_boolean(POWER_SCHEMA_ENABLE_CHARGE_LOW_SAVER);
    if (charge_low_saver_enabled)
    {
        PowerSave::get_instance()->do_cpu_saver();
    }
}

void PowerEventControl::charge_action_event(std::shared_ptr<PowerUPowerDevice> device)
{
    const UPowerDeviceProps& device_props = device->get_props();

    // 如果电池电量过低，但是未使用则忽略
    if (device_props.type == UP_DEVICE_KIND_BATTERY &&
        !this->upower_client_->get_on_battery())
    {
        return;
    }

    PowerAction action = PowerAction::POWER_ACTION_NOTHING;

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

    // 电量过低执行节能操作，延时执行让用户处理一些紧急事务，notification模块中会进行提示
    auto timeout = Glib::MainContext::get_default()->signal_timeout();
    timeout.connect_seconds(sigc::bind(sigc::mem_fun(this, &PowerEventControl::do_charge_critical_action), action),
                            POWER_CRITICAL_ACTION_DELAY);
}

bool PowerEventControl::do_charge_critical_action(PowerAction action)
{
    std::string error;
    if (!PowerSave::get_instance()->do_save(action, error))
    {
        KLOG_WARNING_POWER("%s", error.c_str());
    }
    return false;
}

void PowerEventControl::on_button_changed(PowerEvent evnet)
{
    std::string error;
    PowerAction action = PowerAction::POWER_ACTION_NOTHING;

    switch (evnet)
    {
    case POWER_EVENT_RELEASE_POWEROFF:
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
        KLOG_WARNING_POWER("%s", error.c_str());
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
    switch (event)
    {
    case UPowerDeviceEvent::UPOWER_DEVICE_EVENT_CHARGING:
        this->charging_event();
        break;
    case UPowerDeviceEvent::UPOWER_DEVICE_EVENT_DISCHARGING:
        this->discharging_event(device);
        break;
    case UPowerDeviceEvent::UPOWER_DEVICE_EVENT_CHARGE_LOW:
        this->charge_low_event(device);
        break;
    case UPowerDeviceEvent::UPOWER_DEVICE_EVENT_CHARGE_ACTION:
        this->charge_action_event(device);
        break;
    default:
        break;
    }
}

}  // namespace Kiran