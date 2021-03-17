/**
 * @file          /kiran-cc-daemon/plugins/power/backlight/power-backlight.h
 * @brief         背光设备的亮度控制管理
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#pragma once

#include "plugins/power/backlight/power-backlight-device.h"
#include "power_i.h"

namespace Kiran
{
class PowerBacklight
{
public:
    PowerBacklight();
    virtual ~PowerBacklight();

    static PowerBacklight* get_instance() { return instance_; };

    static void global_init();

    static void global_deinit() { delete instance_; };

    std::shared_ptr<PowerBacklightDevice> get_backlight_device(PowerDeviceType device);

private:
    void init();

private:
    static PowerBacklight* instance_;

    std::shared_ptr<PowerBacklightDevice> backlight_kbd_;
    std::shared_ptr<PowerBacklightDevice> backlight_monitor_;
};
}  // namespace Kiran