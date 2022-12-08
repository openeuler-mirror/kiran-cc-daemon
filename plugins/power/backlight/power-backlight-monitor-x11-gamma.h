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

#include <gdkmm.h>
//
#include <X11/extensions/Xrandr.h>
#include <gdk/gdkx.h>

#include "plugins/power/backlight/power-backlight-interface.h"

namespace Kiran
{
struct GammaInfo
{
    GammaInfo() : brightness(0.0), red(1.0), green(1.0), blue(1.0) {}
    double brightness;
    double red;
    double green;
    double blue;
};

// 通过Xrandr扩展调节crtc的gamma值来实现亮度变化
class PowerBacklightMonitorX11Gamma : public PowerBacklightAbsolute
{
public:
    PowerBacklightMonitorX11Gamma(RROutput output, RRCrtc crtc);
    virtual ~PowerBacklightMonitorX11Gamma(){};

    // 设置亮度值
    virtual bool set_brightness_value(int32_t brightness_value) override;
    // 获取亮度值
    virtual int32_t get_brightness_value() override;
    // 获取亮度最大最小值
    virtual bool get_brightness_range(int32_t &min, int32_t &max) override;

private:
    int find_last_non_clamped(unsigned short array[], int size);
    GammaInfo get_gamma_info();

private:
    GdkDisplay *display_;
    Display *xdisplay_;
    RROutput output_;
    RRCrtc crtc_;
};

}  // namespace Kiran
