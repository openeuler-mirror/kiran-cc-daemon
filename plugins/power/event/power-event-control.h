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

    // 执行正在充电事件
    void charging_event();
    // 执行放电事件
    void discharging_event(std::shared_ptr<PowerUPowerDevice> device);
    // 执行电量过低事件
    void charge_low_event(std::shared_ptr<PowerUPowerDevice> device);
    // 执行电量不足事件
    void charge_action_event(std::shared_ptr<PowerUPowerDevice> device);
    bool do_charge_critical_action(PowerAction action);

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
    // 设置显示器变暗成功
    bool display_dimmed_set_;
};
}  // namespace Kiran