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
#include "plugins/power/event/power-event-button.h"
#include "plugins/power/wrapper/power-wrapper-manager.h"

namespace Kiran
{
// 处理按键事件和电源电量变化事件
class PowerEventControl
{
public:
    PowerEventControl() = delete;
    PowerEventControl(PowerWrapperManager* wrapper_manager, PowerBacklight* backlight);
    virtual ~PowerEventControl();

    static PowerEventControl* get_instance() { return instance_; };

    static void global_init(PowerWrapperManager* wrapper_manager, PowerBacklight* backlight);

    static void global_deinit() { delete instance_; };

private:
    void init();

    bool do_critical_action(PowerAction action);

    void on_button_changed(PowerEvent evnet);
    void on_kbd_brightness_changed(int32_t brightness_value);
    void on_device_status_changed(std::shared_ptr<PowerUPowerDevice> device, UPowerDeviceEvent event);

private:
    static PowerEventControl* instance_;

    PowerWrapperManager* wrapper_manager_;
    std::shared_ptr<PowerUPower> upower_client_;
    std::shared_ptr<PowerScreenSaver> screensaver_;

    PowerBacklight* backlight_;
    std::shared_ptr<PowerBacklightPercentage> backlight_kbd_;
    std::shared_ptr<PowerBacklightPercentage> backlight_monitor_;
    // 键盘上一次设置的非0值
    int32_t kbd_last_nozero_brightness_;

    uint32_t lid_closed_throttle_;

    Glib::RefPtr<Gio::Settings> power_settings_;

    PowerEventButton event_button_;
};
}  // namespace Kiran