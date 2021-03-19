/**
 * @file          /kiran-cc-daemon/plugins/power/backlight/power-backlight-monitor-tool.h
 * @brief         直接通过读写配置文件调节亮度值
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#pragma once

#include "plugins/power/backlight/power-backlight-base.h"

namespace Kiran
{
class PowerBacklightMonitorTool : public PowerBacklightAbsolute
{
public:
    PowerBacklightMonitorTool();
    virtual ~PowerBacklightMonitorTool(){};

    // 设置亮度值
    virtual bool set_brightness_value(int32_t brightness_value) override;
    // 获取亮度值
    virtual int32_t get_brightness_value() override;
    // 获取亮度最大最小值
    virtual bool get_brightness_range(int32_t &min, int32_t &max) override;
};
}  // namespace Kiran
