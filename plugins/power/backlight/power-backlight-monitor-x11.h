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

#include <gdkmm.h>
//
#include <X11/extensions/Xrandr.h>
#include <gdk/gdkx.h>

#include "plugins/power/backlight/power-backlight-base.h"

namespace Kiran
{
// 通过Xrandr扩展调节单个显示设备亮度值
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
