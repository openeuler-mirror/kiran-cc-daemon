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

#include "plugins/power/save/power-save.h"
#include "plugins/power/power-utils.h"

namespace Kiran
{
// 亮度变暗需要一个过程，这里预估为10秒以内
#define DISPLAY_DIMMED_INTERVAL 10
#define CPU_SAVER_INTERVAL 3

PowerSave::PowerSave(PowerWrapperManager* wrapper_manager,
                     PowerBacklight* backlight) : wrapper_manager_(wrapper_manager),
                                                  backlight_(backlight),
                                                  kbd_restore_brightness_(-1),
                                                  monitor_restore_brightness_(-1),
                                                  display_dimmed_timestamp_(0),
                                                  cpu_saver_cookie_(0),
                                                  cpu_saver_timestamp_(0)
{
    this->power_settings_ = Gio::Settings::create(POWER_SCHEMA_ID);
    this->backlight_kbd_ = backlight->get_backlight_device(PowerDeviceType::POWER_DEVICE_TYPE_KBD);
    this->backlight_monitor_ = backlight->get_backlight_device(PowerDeviceType::POWER_DEVICE_TYPE_MONITOR);
    this->profiles_ = this->wrapper_manager_->get_default_profiles();
}

PowerSave::~PowerSave()
{
}

PowerSave* PowerSave::instance_ = nullptr;
void PowerSave::global_init(PowerWrapperManager* wrapper_manager, PowerBacklight* backlight)
{
    instance_ = new PowerSave(wrapper_manager, backlight);
    instance_->init();
}

bool PowerSave::do_save(PowerAction action, std::string& error)
{
    KLOG_DEBUG("Do power save action '%s'.", PowerUtils::action_enum2str(action).c_str());

    switch (action)
    {
    case PowerAction::POWER_ACTION_DISPLAY_ON:
        this->save_dpms_.set_level(PowerDpmsLevel::POWER_DPMS_LEVEL_ON);
        break;
    case PowerAction::POWER_ACTION_DISPLAY_STANDBY:
        this->save_dpms_.set_level(PowerDpmsLevel::POWER_DPMS_LEVEL_STANDBY);
        break;
    case PowerAction::POWER_ACTION_DISPLAY_SUSPEND:
        this->save_dpms_.set_level(PowerDpmsLevel::POWER_DPMS_LEVEL_SUSPEND);
        break;
    case PowerAction::POWER_ACTION_DISPLAY_OFF:
        this->save_dpms_.set_level(PowerDpmsLevel::POWER_DPMS_LEVEL_OFF);
        break;
    case PowerAction::POWER_ACTION_COMPUTER_SUSPEND:
        this->save_computer_.suspend();
        break;
    case PowerAction::POWER_ACTION_COMPUTER_SHUTDOWN:
        this->save_computer_.shutdown();
        break;
    case PowerAction::POWER_ACTION_COMPUTER_HIBERNATE:
        this->save_computer_.hibernate();
        break;
    case PowerAction::POWER_ACTION_NOTHING:
        break;
    default:
        error = "Unsupported action";
        return false;
    }
    return true;
}

bool PowerSave::is_display_dimmed()
{
    if (this->kbd_restore_brightness_ != -1 || this->monitor_restore_brightness_ != -1)
    {
        return true;
    }
    return false;
}

bool PowerSave::do_display_dimmed()
{
    // 如果还处于变暗状态，则不允许重新设置，避免多个场景(计算机空闲、电量过低）下设置变暗->恢复变暗冲突
    if (this->is_display_dimmed())
    {
        KLOG_DEBUG("The display already is dimmed status.");
        return false;
    }

    auto brightness_percentage = this->power_settings_->get_int(POWER_SCHEMA_DISPLAY_DIMMED_BRIGHTNESS);
    if (brightness_percentage > 0 && brightness_percentage <= 100)
    {
        this->display_dimmed_timestamp_ = time(NULL);

        auto kbd_brightness_percentage = this->backlight_kbd_->get_brightness();
        if (kbd_brightness_percentage >= 0)
        {
            this->backlight_kbd_->set_brightness(brightness_percentage);
            this->kbd_restore_brightness_ = kbd_brightness_percentage;
        }

        auto monitor_brightness_percentage = this->backlight_monitor_->get_brightness();
        if (monitor_brightness_percentage >= 0)
        {
            this->backlight_monitor_->set_brightness(brightness_percentage);
            this->monitor_restore_brightness_ = monitor_brightness_percentage;
        }

        KLOG_DEBUG("The display is dimmed.");
    }
    else
    {
        KLOG_WARNING("The brightness value is invalid: %d", brightness_percentage);
    }

    return true;
}

void PowerSave::do_display_restore_dimmed()
{
    RETURN_IF_TRUE(!this->is_display_dimmed());

    auto kbd_brightness_percentage = this->backlight_kbd_->get_brightness();

    if (kbd_brightness_percentage >= 0 && this->kbd_restore_brightness_ >= 0)
    {
        this->backlight_kbd_->set_brightness(this->kbd_restore_brightness_);
        this->kbd_restore_brightness_ = -1;
    }

    auto monitor_brightness_percentage = this->backlight_monitor_->get_brightness();
    if (monitor_brightness_percentage >= 0 && this->monitor_restore_brightness_ >= 0)
    {
        this->backlight_monitor_->set_brightness(this->monitor_restore_brightness_);
        this->monitor_restore_brightness_ = -1;
    }

    this->display_dimmed_timestamp_ = 0;

    KLOG_DEBUG("The display is restore dimmed.");
}

void PowerSave::do_cpu_saver()
{
    if (this->cpu_saver_cookie_ > 0)
    {
        KLOG_DEBUG("The cpu already is on saver mode.");
        return;
    }

    this->cpu_saver_cookie_ = this->profiles_->hold_profile(POWER_PROFILE_SAVER, "battery or ups power low.", "kiran-session-daemon");
    this->cpu_saver_timestamp_ = time(NULL);
}

void PowerSave::do_cpu_restore_saver()
{
    if (this->cpu_saver_cookie_ > 0)
    {
        this->profiles_->release_profile(this->cpu_saver_cookie_);
        this->cpu_saver_cookie_ = 0;
    }
    this->cpu_saver_timestamp_ = 0;
}

void PowerSave::init()
{
    this->save_computer_.init();
    this->save_dpms_.init();

    this->backlight_kbd_->signal_brightness_changed().connect(sigc::mem_fun(this, &PowerSave::on_kbd_brightness_changed));
    this->backlight_monitor_->signal_brightness_changed().connect(sigc::mem_fun(this, &PowerSave::on_monitor_brightness_changed));
    this->profiles_->signal_active_profile_changed().connect(sigc::mem_fun(this, &PowerSave::on_active_profile_changed));
}

void PowerSave::on_kbd_brightness_changed(int32_t brightness_percentage)
{
    // 亮度变暗操作结束DISPLAY_DIMMED_INTERVAL秒后，认为后续的亮度变化为手动设置，如果亮度进行了手动设置，则不再做亮度变暗恢复操作。
    if (this->display_dimmed_timestamp_ > 0 &&
        this->display_dimmed_timestamp_ + DISPLAY_DIMMED_INTERVAL < time(NULL))
    {
        KLOG_DEBUG("The keyboard brightness is changed, so ignore keyboard brightness restores.");
        this->kbd_restore_brightness_ = -1;
    }
}

void PowerSave::on_monitor_brightness_changed(int32_t brightness_percentage)
{
    if (this->display_dimmed_timestamp_ > 0 &&
        this->display_dimmed_timestamp_ + DISPLAY_DIMMED_INTERVAL < time(NULL))
    {
        KLOG_DEBUG("The monitor brightness is changed, so ignore monitor brightness restores.");
        this->monitor_restore_brightness_ = -1;
    }
}

void PowerSave::on_active_profile_changed(const Glib::ustring& active_profile)
{
    if (this->cpu_saver_timestamp_ > 0 &&
        this->cpu_saver_timestamp_ + CPU_SAVER_INTERVAL < time(NULL))
    {
        KLOG_DEBUG("The power active profile is changed, so release previous profile.");
        this->do_cpu_restore_saver();
    }
}
}  // namespace Kiran