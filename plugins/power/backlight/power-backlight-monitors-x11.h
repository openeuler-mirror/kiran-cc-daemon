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

#include <gdkmm.h>
//
#include <X11/extensions/Xrandr.h>
#include <gdk/gdkx.h>

#include "plugins/power/backlight/power-backlight-interface.h"

namespace Kiran
{
class PowerBacklightMonitorsX11 : public PowerBacklightMonitors
{
public:
    PowerBacklightMonitorsX11();
    virtual ~PowerBacklightMonitorsX11();

    virtual void init();
    // 获取所有显示器亮度设置对象
    virtual PowerBacklightAbsoluteVec get_monitors() { return this->backlight_monitors_; }
    virtual sigc::signal<void> signal_monitor_changed() { return this->monitor_changed_; };
    virtual sigc::signal<void> signal_brightness_changed() { return this->brightness_changed_; }

private:
    bool init_xrandr();
    Atom get_backlight_atom();

    // 是否支持设置亮度
    bool support_backlight_extension() { return this->extension_supported_; };

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

    PowerBacklightAbsoluteVec backlight_monitors_;

    sigc::signal<void> monitor_changed_;
    sigc::signal<void> brightness_changed_;
};
}  // namespace Kiran