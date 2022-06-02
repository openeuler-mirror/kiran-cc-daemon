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

#include "plugins/power/idle/power-idle-control.h"

#include "plugins/power/save/power-save.h"

namespace Kiran
{
PowerIdleControl::PowerIdleControl(PowerWrapperManager* wrapper_manager,
                                   PowerBacklight* backlight) : wrapper_manager_(wrapper_manager),
                                                                backlight_(backlight),
                                                                computer_idle_time_(0),
                                                                display_idle_time_(0),
                                                                kbd_normal_brightness_(-1),
                                                                monitor_normal_brightness_(-1)
{
    this->upower_client_ = this->wrapper_manager_->get_default_upower();
    this->backlight_kbd_ = backlight->get_backlight_device(PowerDeviceType::POWER_DEVICE_TYPE_KBD);
    this->backlight_monitor_ = backlight->get_backlight_device(PowerDeviceType::POWER_DEVICE_TYPE_MONITOR);
    power_settings_ = Gio::Settings::create(POWER_SCHEMA_ID);
}

PowerIdleControl::~PowerIdleControl()
{
}

PowerIdleControl* PowerIdleControl::instance_ = nullptr;
void PowerIdleControl::global_init(PowerWrapperManager* wrapper_manager, PowerBacklight* backlight)
{
    instance_ = new PowerIdleControl(wrapper_manager, backlight);
    instance_->init();
}

void PowerIdleControl::init()
{
    this->idle_timer_.init();
    this->update_idle_timer();

    this->kbd_normal_brightness_ = this->backlight_kbd_->get_brightness();
    this->monitor_normal_brightness_ = this->backlight_monitor_->get_brightness();

    this->upower_client_->signal_on_battery_changed().connect(sigc::mem_fun(this, &PowerIdleControl::on_battery_changed));
    this->power_settings_->signal_changed().connect(sigc::mem_fun(this, &PowerIdleControl::on_settings_changed));
    this->idle_timer_.signal_mode_changed().connect(sigc::mem_fun(this, &PowerIdleControl::on_idle_mode_changed));
    this->backlight_kbd_->signal_brightness_changed().connect(sigc::mem_fun(this, &PowerIdleControl::on_kbd_brightness_changed));
    this->backlight_monitor_->signal_brightness_changed().connect(sigc::mem_fun(this, &PowerIdleControl::on_monitor_brightness_changed));
}

void PowerIdleControl::update_idle_timer()
{
    if (this->upower_client_->get_on_battery())
    {
        this->computer_idle_time_ = this->power_settings_->get_int(POWER_SCHEMA_COMPUTER_BATTERY_IDLE_TIME);
        this->computer_idle_action_ = PowerAction(this->power_settings_->get_enum(POWER_SCHEMA_COMPUTER_BATTERY_IDLE_ACTION));

        this->display_idle_time_ = this->power_settings_->get_int(POWER_SCHEMA_BACKLIGHT_BATTERY_IDLE_TIME);
        this->display_idle_action_ = PowerAction(this->power_settings_->get_enum(POWER_SCHEMA_BACKLIGHT_BATTERY_IDLE_ACTION));
    }
    else
    {
        this->computer_idle_time_ = this->power_settings_->get_int(POWER_SCHEMA_COMPUTER_AC_IDLE_TIME);
        this->computer_idle_action_ = PowerAction(this->power_settings_->get_enum(POWER_SCHEMA_COMPUTER_AC_IDLE_ACTION));

        this->display_idle_time_ = this->power_settings_->get_int(POWER_SCHEMA_BACKLIGHT_AC_IDLE_TIME);
        this->display_idle_action_ = PowerAction(this->power_settings_->get_enum(POWER_SCHEMA_BACKLIGHT_AC_IDLE_ACTION));
    }

    this->idle_timer_.set_idle_timeout(PowerIdleMode::POWER_IDLE_MODE_BLANK, this->display_idle_time_);
    this->idle_timer_.set_idle_timeout(PowerIdleMode::POWER_IDLE_MODE_SLEEP, this->computer_idle_time_);
}

void PowerIdleControl::switch_to_normal()
{
    std::string error;

    // 正常状态下退出显示器的节能模式
    if (!PowerSave::get_instance()->do_save(PowerAction::POWER_ACTION_DISPLAY_ON, error))
    {
        KLOG_WARNING("%s", error.c_str());
    }

    // 切换到键盘上一次处于正常模式下的亮度值，以确保从节能模式恢复到正常模式后亮度值也能恢复到之前的值
    this->backlight_kbd_->set_brightness(this->kbd_normal_brightness_);
    this->backlight_monitor_->set_brightness(this->monitor_normal_brightness_);
}

void PowerIdleControl::switch_to_dim()
{
    auto scale = this->power_settings_->get_int(POWER_SCHEMA_DISPLAY_IDLE_DIM_SCALE);
    if (scale > 0 && scale <= 100)
    {
        auto kbd_brightness_percentage = this->backlight_kbd_->get_brightness();
        if (kbd_brightness_percentage > 0)
        {
            this->backlight_kbd_->set_brightness(kbd_brightness_percentage * (100 - scale) / 100);
        }

        auto monitor_brightness_percentage = this->backlight_monitor_->get_brightness();
        if (monitor_brightness_percentage)
        {
            this->backlight_monitor_->set_brightness(monitor_brightness_percentage * (100 - scale) / 100);
        }
    }
    else if (scale < 0 || scale > 100)
    {
        KLOG_WARNING("The scale is exceed limit. scale: %d.", scale);
    }
}

void PowerIdleControl::switch_to_blank()
{
    std::string error;

    if (!PowerSave::get_instance()->do_save(this->display_idle_action_, error))
    {
        KLOG_WARNING("%s", error.c_str());
    }
    // 黑屏时将键盘亮度设置为0
    this->backlight_kbd_->set_brightness(0);
}

void PowerIdleControl::switch_to_sleep()
{
    std::string error;

    if (!PowerSave::get_instance()->do_save(this->computer_idle_action_, error))
    {
        KLOG_WARNING("%s", error.c_str());
    }
}

void PowerIdleControl::on_battery_changed(bool)
{
    // 电池/电源切换时，空闲超时参数需要重新设置
    this->update_idle_timer();
}

void PowerIdleControl::on_settings_changed(const Glib::ustring& key)
{
    switch (shash(key.c_str()))
    {
    case CONNECT(POWER_SCHEMA_COMPUTER_BATTERY_IDLE_TIME, _hash):
    case CONNECT(POWER_SCHEMA_COMPUTER_BATTERY_IDLE_ACTION, _hash):
    case CONNECT(POWER_SCHEMA_BACKLIGHT_BATTERY_IDLE_TIME, _hash):
    case CONNECT(POWER_SCHEMA_BACKLIGHT_BATTERY_IDLE_ACTION, _hash):
    case CONNECT(POWER_SCHEMA_COMPUTER_AC_IDLE_TIME, _hash):
    case CONNECT(POWER_SCHEMA_COMPUTER_AC_IDLE_ACTION, _hash):
    case CONNECT(POWER_SCHEMA_BACKLIGHT_AC_IDLE_TIME, _hash):
    case CONNECT(POWER_SCHEMA_BACKLIGHT_AC_IDLE_ACTION, _hash):
        this->update_idle_timer();
        break;
    }
}

void PowerIdleControl::on_idle_mode_changed(PowerIdleMode mode)
{
    KLOG_PROFILE("mode: %d", mode);

    std::string error;

    switch (mode)
    {
    case PowerIdleMode::POWER_IDLE_MODE_NORMAL:
        this->switch_to_normal();
        break;
    case PowerIdleMode::POWER_IDLE_MODE_DIM:
        this->switch_to_dim();
        break;
    case PowerIdleMode::POWER_IDLE_MODE_BLANK:
        this->switch_to_blank();
        break;
    case PowerIdleMode::POWER_IDLE_MODE_SLEEP:
        this->switch_to_sleep();
        break;
    default:
        break;
    }
}

void PowerIdleControl::on_kbd_brightness_changed(int32_t brightness_percentage)
{
    if (this->idle_timer_.get_idle_mode() == PowerIdleMode::POWER_IDLE_MODE_NORMAL)
    {
        this->kbd_normal_brightness_ = brightness_percentage;
    }
}

void PowerIdleControl::on_monitor_brightness_changed(int32_t brightness_percentage)
{
    if (this->idle_timer_.get_idle_mode() == PowerIdleMode::POWER_IDLE_MODE_NORMAL)
    {
        this->monitor_normal_brightness_ = brightness_percentage;
    }
}
}  // namespace Kiran