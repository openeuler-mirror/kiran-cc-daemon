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
                                                                display_dimmed_set_(false)
{
    this->upower_client_ = this->wrapper_manager_->get_default_upower();
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

    this->upower_client_->signal_on_battery_changed().connect(sigc::mem_fun(this, &PowerIdleControl::on_battery_changed));
    this->power_settings_->signal_changed().connect(sigc::mem_fun(this, &PowerIdleControl::on_settings_changed));
    this->idle_timer_.signal_mode_changed().connect(sigc::mem_fun(this, &PowerIdleControl::on_idle_mode_changed));
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
        KLOG_WARNING_POWER("%s", error.c_str());
    }

    // 之前如果设置过变暗操作，则进行恢复
    if (this->display_dimmed_set_)
    {
        PowerSave::get_instance()->do_display_restore_dimmed();
        this->display_dimmed_set_ = false;
    }
}

void PowerIdleControl::switch_to_dim()
{
    auto display_idle_dimmed_enabled = this->power_settings_->get_boolean(POWER_SCHEMA_ENABLE_DISPLAY_IDLE_DIMMED);
    // 这里必须要判断当前是否处于变暗状态。如果当前已经处于变暗状态，调用do_display_dimmed函数会导致display_dimmed_set_置为false。
    if (display_idle_dimmed_enabled && !PowerSave::get_instance()->is_display_dimmed())
    {
        this->display_dimmed_set_ = PowerSave::get_instance()->do_display_dimmed();
    }
}

void PowerIdleControl::switch_to_blank()
{
    std::string error;

    if (!PowerSave::get_instance()->do_save(this->display_idle_action_, error))
    {
        KLOG_WARNING_POWER("%s", error.c_str());
    }
}

void PowerIdleControl::switch_to_sleep()
{
    std::string error;

    if (!PowerSave::get_instance()->do_save(this->computer_idle_action_, error))
    {
        KLOG_WARNING_POWER("%s", error.c_str());
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

}  // namespace Kiran