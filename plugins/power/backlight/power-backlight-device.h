/**
 * @file          /kiran-cc-daemon/plugins/power/backlight/power-backlight-device.h
 * @brief         背光设备亮度控制基类
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#pragma once

#include "lib/base/base.h"

namespace Kiran
{
class PowerBacklightDevice
{
public:
    PowerBacklightDevice(){};
    virtual ~PowerBacklightDevice(){};

    virtual void init() = 0;

    // 设置亮度百分比
    virtual bool set_brightness(int32_t percentage) = 0;
    // 获取亮度百分比，如果小于0，则说明不支持调节亮度
    virtual int32_t get_brightness() = 0;

    // 增加亮度
    virtual bool brightness_up() = 0;
    // 降低亮度
    virtual bool brightness_down() = 0;

    // 亮度发生变化
    virtual sigc::signal<void, int32_t> &signal_brightness_changed() = 0;
};

class PowerBacklightAbsolute
{
public:
    PowerBacklightAbsolute(){};
    virtual ~PowerBacklightAbsolute(){};

    // 设置亮度值
    virtual bool set_brightness_value(int32_t brightness_value) = 0;
    // 获取亮度值
    virtual int32_t get_brightness_value() = 0;
    // 获取亮度最大最小值
    virtual bool get_brightness_range(int32_t &min, int32_t &max) = 0;
};

using PowerBacklightAbsoluteVec = std::vector<std::shared_ptr<PowerBacklightAbsolute>>;
}  // namespace Kiran