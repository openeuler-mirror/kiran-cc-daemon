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