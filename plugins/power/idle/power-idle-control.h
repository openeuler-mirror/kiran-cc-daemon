/**
 * @file          /kiran-cc-daemon/plugins/power/idle/power-idle-control.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#pragma once

#include "plugins/power/backlight/power-backlight.h"
#include "plugins/power/idle/power-idle-timer.h"
#include "plugins/power/wrapper/power-wrapper-manager.h"
#include "power_i.h"

namespace Kiran
{
class PowerIdleControl
{
public:
    PowerIdleControl() = delete;
    PowerIdleControl(PowerWrapperManager* wrapper_manager, PowerBacklight* backlight);
    virtual ~PowerIdleControl();

    static PowerIdleControl* get_instance() { return instance_; };

    static void global_init(PowerWrapperManager* wrapper_manager, PowerBacklight* backlight);

    static void global_deinit() { delete instance_; };

private:
    void init();

    void update_idle_timer();

    void switch_to_normal();
    void switch_to_dim();
    void switch_to_blank();
    void switch_to_sleep();

    void on_battery_changed(bool);
    void on_settings_changed(const Glib::ustring& key);
    void on_idle_mode_changed(PowerIdleMode mode);
    void on_kbd_brightness_changed(int32_t brightness_percentage);

private:
    static PowerIdleControl* instance_;

    PowerWrapperManager* wrapper_manager_;
    PowerBacklight* backlight_;

    PowerIdleTimer idle_timer_;
    Glib::RefPtr<Gio::Settings> power_settings_;

    std::shared_ptr<PowerUPower> upower_client_;
    std::shared_ptr<PowerBacklightDevice> backlight_kbd_;
    std::shared_ptr<PowerBacklightDevice> backlight_monitor_;

    int32_t computer_idle_time_;
    PowerAction computer_idle_action_;

    int32_t display_idle_time_;
    PowerAction display_idle_action_;

    // 上一次处于正常模式时键盘的亮度百分比
    int32_t kbd_brightness_percentage_;
};
}  // namespace Kiran