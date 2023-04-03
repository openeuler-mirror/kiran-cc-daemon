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

    int32_t computer_idle_time_;
    PowerAction computer_idle_action_;

    int32_t display_idle_time_;
    PowerAction display_idle_action_;

    // 设置显示器变暗成功
    bool display_dimmed_set_;
};
}  // namespace Kiran