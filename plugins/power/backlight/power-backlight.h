/**
 * @file          /kiran-cc-daemon/plugins/power/backlight/power-backlight.h
 * @brief         背光设备的亮度控制管理
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#pragma once

#include "plugins/power/backlight/power-backlight-base.h"
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

    std::shared_ptr<PowerBacklightPercentage> get_backlight_device(PowerDeviceType device);

    // 背光设备亮度发生变化
    sigc::signal<void, std::shared_ptr<PowerBacklightPercentage>, int32_t>& signal_brightness_changed() { return this->brightness_changed_; };

private:
    void init();

    void on_backlight_brightness_changed(int32_t brightness_percentage, std::shared_ptr<PowerBacklightPercentage> backlight);

private:
    static PowerBacklight* instance_;

    std::shared_ptr<PowerBacklightPercentage> backlight_kbd_;
    std::shared_ptr<PowerBacklightPercentage> backlight_monitor_;

    sigc::signal<void, std::shared_ptr<PowerBacklightPercentage>, int32_t> brightness_changed_;
};
}  // namespace Kiran