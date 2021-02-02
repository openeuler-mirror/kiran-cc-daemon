/**
 * @file          /kiran-cc-daemon/plugins/power/backlight/power-backlight-monitor-x11.h
 * @brief         通过Xrandr扩展调节单个显示设备亮度值
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */
#pragma once

#include <gdkmm.h>
//
#include <X11/extensions/Xrandr.h>
#include <gdk/gdkx.h>

#include "plugins/power/backlight/power-backlight-device.h"

namespace Kiran
{
class PowerBacklightMonitorX11 : public PowerBacklightAbsolute
{
public:
    PowerBacklightMonitorX11(Atom backlight_atom, RROutput output);
    virtual ~PowerBacklightMonitorX11(){};

    // 设置亮度值
    virtual bool set_brightness_value(int32_t brightness_value) override;
    // 获取亮度值
    virtual int32_t get_brightness_value() override;
    // 获取亮度最大最小值
    virtual bool get_brightness_range(int32_t &min, int32_t &max) override;

private:
    GdkDisplay *display_;
    Display *xdisplay_;
    Atom backlight_atom_;
    RROutput output_;
};

using PowerBacklightMonitorX11Vec = std::vector<std::shared_ptr<PowerBacklightMonitorX11>>;
}  // namespace Kiran
