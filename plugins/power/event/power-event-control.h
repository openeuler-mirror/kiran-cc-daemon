/**
 * @file          /kiran-cc-daemon/plugins/power/event/power-event-control.h
 * @brief         处理按键事件和电源电量变化事件
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#pragma once

#include "plugins/power/backlight/power-backlight.h"
#include "plugins/power/event/power-event-button.h"
#include "plugins/power/wrapper/power-wrapper-manager.h"

namespace Kiran
{
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
    std::shared_ptr<PowerBacklightDevice> backlight_kbd_;
    std::shared_ptr<PowerBacklightDevice> backlight_monitor_;
    // 键盘上一次设置的非0值
    int32_t kbd_last_nozero_brightness_;

    uint32_t lid_closed_throttle_;

    Glib::RefPtr<Gio::Settings> power_settings_;

    PowerEventButton event_button_;
};
}  // namespace Kiran