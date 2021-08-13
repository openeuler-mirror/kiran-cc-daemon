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

#include "plugins/power/backlight/power-backlight-x11.h"

namespace Kiran
{
PowerBacklightX11::PowerBacklightX11() : event_base_(0),
                                         error_base_(0),
                                         extension_supported_(false),
                                         backlight_atom_(None),
                                         resources_(NULL)
{
    this->display_ = gdk_display_get_default();
    this->xdisplay_ = GDK_DISPLAY_XDISPLAY(this->display_);

    auto screen = gdk_screen_get_default();
    this->root_window_ = gdk_screen_get_root_window(screen);
    this->xroot_window_ = GDK_WINDOW_XID(this->root_window_);
}

PowerBacklightX11::~PowerBacklightX11()
{
    this->clear_resource();

    if (this->extension_supported_)
    {
        gdk_window_remove_filter(this->root_window_, &PowerBacklightX11::window_event, this);
    }
}

void PowerBacklightX11::init()
{
    RETURN_IF_FALSE(this->init_xrandr());

    this->backlight_atom_ = this->get_backlight_atom();
    RETURN_IF_TRUE(this->backlight_atom_ == None);

    KLOG_DEBUG("Support brightness settings");
    this->load_resource();

    XRRSelectInput(this->xdisplay_, this->xroot_window_, RRScreenChangeNotifyMask | RROutputPropertyNotifyMask);
    gdk_x11_register_standard_event_type(this->display_,
                                         this->event_base_,
                                         RRNotify + 1);

    gdk_window_add_filter(this->root_window_, &PowerBacklightX11::window_event, this);
    this->extension_supported_ = true;
}

bool PowerBacklightX11::init_xrandr()
{
    KLOG_PROFILE("");

    if (XRRQueryExtension(this->xdisplay_, &this->event_base_, &this->error_base_))
    {
        int major_version = 0;
        int minor_version = 0;
        XRRQueryVersion(this->xdisplay_, &major_version, &minor_version);
        if (major_version < 1 || (major_version == 1 && minor_version < 3))
        {
            KLOG_WARNING("RANDR extension is too old (must be at least 1.3). current version: %d:%d.",
                         major_version,
                         minor_version);
            return false;
        }
    }
    else
    {
        KLOG_WARNING("RANDR extension is not present");
        return false;
    }
    return true;
}

Atom PowerBacklightX11::get_backlight_atom()
{
    RETURN_VAL_IF_TRUE(this->xdisplay_ == NULL, false);

    // 此属性适用于笔记本电脑和带背光控制器的显示器
    auto backlight_atom = XInternAtom(this->xdisplay_, "Backlight", True);
    if (backlight_atom == None)
    {
        // 兼容老的属性
        backlight_atom = XInternAtom(this->xdisplay_, "BACKLIGHT", True);
        if (backlight_atom == None)
        {
            KLOG_DEBUG("No outputs have backlight property");
            return None;
        }
    }
    return backlight_atom;
}

void PowerBacklightX11::load_resource()
{
    this->clear_resource();
    this->resources_ = XRRGetScreenResourcesCurrent(this->xdisplay_, this->xroot_window_);

    this->backlight_monitors_.clear();
    for (int32_t i = 0; i < this->resources_->noutput; ++i)
    {
        auto monitor = std::make_shared<PowerBacklightMonitorX11>(this->backlight_atom_, this->resources_->outputs[i]);
        this->backlight_monitors_.push_back(monitor);
    }
}

void PowerBacklightX11::clear_resource()
{
    if (this->resources_)
    {
        XRRFreeScreenResources(this->resources_);
        this->resources_ = NULL;
    }
}

GdkFilterReturn PowerBacklightX11::window_event(GdkXEvent *gdk_event, GdkEvent *event, gpointer data)
{
    PowerBacklightX11 *backlight = (PowerBacklightX11 *)data;

    XEvent *xevent = (XEvent *)gdk_event;
    RETURN_VAL_IF_FALSE(backlight, GDK_FILTER_CONTINUE);
    RETURN_VAL_IF_FALSE(xevent, GDK_FILTER_CONTINUE);

    switch (xevent->type - backlight->event_base_)
    {
    case RRScreenChangeNotify:
    {
        backlight->load_resource();
        backlight->monitor_changed_.emit(PBXMonitorEvent::PBX_MONITOR_EVENT_SCREEN_CHANGED);
        break;
    }
    case RROutputPropertyNotifyMask:
    {
        backlight->monitor_changed_.emit(PBXMonitorEvent::PBX_MONITOR_EVENT_PROPERTY_CHANGED);
        break;
    }
    default:
        return GDK_FILTER_CONTINUE;
    }

    return GDK_FILTER_CONTINUE;
}
}  // namespace Kiran
