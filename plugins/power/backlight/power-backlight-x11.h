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

#include "plugins/power/backlight/power-backlight-monitor-x11.h"

namespace Kiran
{
enum PBXMonitorEvent
{
    // 显示器列表变化
    PBX_MONITOR_EVENT_SCREEN_CHANGED,
    // 显示器属性(亮度)可能发生变化
    PBX_MONITOR_EVENT_PROPERTY_CHANGED,
};

class PowerBacklightX11
{
public:
    PowerBacklightX11();
    virtual ~PowerBacklightX11();

    void init();

    // 是否支持设置亮度
    bool support_backlight_extension() { return this->extension_supported_; };

    // 获取所有显示器亮度设置对象
    PowerBacklightMonitorX11Vec get_monitors() { return this->backlight_monitors_; }

    sigc::signal<void, PBXMonitorEvent> signal_monitor_changed() { return this->monitor_changed_; };

private:
    bool init_xrandr();
    Atom get_backlight_atom();

    void load_resource();
    void clear_resource();

    static GdkFilterReturn window_event(GdkXEvent *gdk_event, GdkEvent *event, gpointer data);

private:
    GdkDisplay *display_;
    Display *xdisplay_;
    GdkWindow *root_window_;
    Window xroot_window_;

    int32_t event_base_;
    int32_t error_base_;
    bool extension_supported_;

    Atom backlight_atom_;
    XRRScreenResources *resources_;

    PowerBacklightMonitorX11Vec backlight_monitors_;

    sigc::signal<void, PBXMonitorEvent> monitor_changed_;
};
}  // namespace Kiran