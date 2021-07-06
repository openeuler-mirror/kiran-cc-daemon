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

#pragma once

#include "plugins/power/backlight/power-backlight.h"
#include "plugins/power/idle/power-idle-timer.h"
#include "plugins/power/wrapper/power-wrapper-manager.h"
#include "power-i.h"

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
    void on_monitor_brightness_changed(int32_t brightness_percentage);

private:
    static PowerIdleControl* instance_;

    PowerWrapperManager* wrapper_manager_;
    PowerBacklight* backlight_;

    PowerIdleTimer idle_timer_;
    Glib::RefPtr<Gio::Settings> power_settings_;

    std::shared_ptr<PowerUPower> upower_client_;
    std::shared_ptr<PowerBacklightPercentage> backlight_kbd_;
    std::shared_ptr<PowerBacklightPercentage> backlight_monitor_;

    int32_t computer_idle_time_;
    PowerAction computer_idle_action_;

    int32_t display_idle_time_;
    PowerAction display_idle_action_;

    // 上一次处于正常模式时键盘的亮度百分比
    int32_t kbd_normal_brightness_;
    int32_t monitor_normal_brightness_;
};
}  // namespace Kiran