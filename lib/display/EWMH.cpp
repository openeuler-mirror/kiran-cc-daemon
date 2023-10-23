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

#include "lib/display/EWMH.h"

#include <X11/Xatom.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>

#include "lib/base/base.h"
namespace Kiran
{
#define NET_SUPPORTING_WM_CHECK "_NET_SUPPORTING_WM_CHECK"
#define NET_WM_NAME "_NET_WM_NAME"
#define MATE_WM_KEYBINDINGS "_MATE_WM_KEYBINDINGS"

EWMH::EWMH() : wm_window_(None)
{
}

EWMH::~EWMH()
{
    gdk_window_remove_filter(NULL, &EWMH::window_event, this);
}

EWMH* EWMH::instance_ = nullptr;
void EWMH::global_init()
{
    RETURN_IF_TRUE(instance_);
    instance_ = new EWMH();
    instance_->init();
}

void EWMH::global_deinit()
{
    delete instance_;
    instance_ = nullptr;
}

std::vector<std::string> EWMH::get_wm_keybindings()
{
    auto keybindings_atom = gdk_x11_get_xatom_by_name(MATE_WM_KEYBINDINGS);
    auto keybindings = this->get_wm_property(keybindings_atom);
    std::vector<std::string> result;

    if (keybindings.length() > 0)
    {
        auto regex = Glib::Regex::create("\\s*,\\s*");
        result = regex->split(keybindings);
    }
    else
    {
        auto wm_atom = gdk_x11_get_xatom_by_name("_NET_WM_NAME");
        auto wm_name = this->get_wm_property(wm_atom);
        if (wm_name.length() > 0)
        {
            result.push_back(wm_name);
        }
    }
    return result;
}

std::string EWMH::get_wm_property(Atom atom)
{
    RETURN_VAL_IF_TRUE(this->wm_window_ == None, std::string());

    std::string retval;
    Atom type;
    int format;
    gulong nitems;
    gulong bytes_after;
    gchar* val = NULL;

    SCOPE_EXIT({if (val) XFree(val); });

    auto utf8_string = gdk_x11_get_xatom_by_name("UTF8_STRING");
    auto display = gdk_display_get_default();

    gdk_x11_display_error_trap_push(display);

    auto result = XGetWindowProperty(GDK_DISPLAY_XDISPLAY(display),
                                     this->wm_window_,
                                     atom,
                                     0,
                                     G_MAXLONG,
                                     False,
                                     utf8_string,
                                     &type,
                                     &format,
                                     &nitems,
                                     &bytes_after,
                                     (guchar**)&val);

    if (!gdk_x11_display_error_trap_pop(display) &&
        result == Success &&
        type == utf8_string &&
        format == 8 &&
        nitems > 0 &&
        g_utf8_validate(val, nitems, NULL))
    {
        retval = std::string(val, nitems);
    }

    return retval;
}

std::string EWMH::get_wm_name()
{
    Atom atom = gdk_x11_get_xatom_by_name(NET_WM_NAME);
    return this->get_wm_property(atom);
}

void EWMH::init()
{
    auto display = Gdk::Display::get_default();
    auto root_window = display->get_default_screen()->get_root_window();

    auto event_mask = root_window->get_events();
    root_window->set_events(event_mask | Gdk::PROPERTY_CHANGE_MASK);
    gdk_window_add_filter(NULL, &EWMH::window_event, this);

    this->update_wm_window();
}

void EWMH::update_wm_window()
{
    Window* xwindow = NULL;
    Atom type;
    gint format;
    gulong nitems;
    gulong bytes_after;

    SCOPE_EXIT({if (xwindow) XFree(xwindow); });

    this->wm_window_ = None;

    auto display = gdk_display_get_default();
    XGetWindowProperty(GDK_DISPLAY_XDISPLAY(display),
                       GDK_ROOT_WINDOW(),
                       gdk_x11_get_xatom_by_name(NET_SUPPORTING_WM_CHECK),
                       0,
                       G_MAXLONG,
                       False,
                       XA_WINDOW,
                       &type,
                       &format,
                       &nitems,
                       &bytes_after,
                       (guchar**)&xwindow);

    RETURN_IF_TRUE(type != XA_WINDOW);

    gdk_x11_display_error_trap_push(display);
    XSelectInput(GDK_DISPLAY_XDISPLAY(display), *xwindow, StructureNotifyMask | PropertyChangeMask);
    XSync(GDK_DISPLAY_XDISPLAY(display), False);

    RETURN_IF_TRUE(gdk_x11_display_error_trap_pop(display));

    this->wm_window_ = *xwindow;
    this->wm_window_changed_.emit();
}

GdkFilterReturn EWMH::window_event(GdkXEvent* gdk_event, GdkEvent* event, gpointer data)
{
    EWMH* manager = (EWMH*)data;
    g_return_val_if_fail(EWMH::get_instance() == manager, GDK_FILTER_REMOVE);
    XEvent* xevent = (XEvent*)gdk_event;

    if ((xevent->type == DestroyNotify &&
         manager->wm_window_ != None &&
         xevent->xany.window == manager->wm_window_) ||
        (xevent->type == PropertyNotify &&
         xevent->xany.window == GDK_ROOT_WINDOW() &&
         xevent->xproperty.atom == (gdk_x11_get_xatom_by_name(NET_SUPPORTING_WM_CHECK))) ||
        (xevent->type == PropertyNotify &&
         manager->wm_window_ != None &&
         xevent->xany.window == manager->wm_window_ &&
         xevent->xproperty.atom == (gdk_x11_get_xatom_by_name(NET_WM_NAME))))
    {
        manager->update_wm_window();
    }

    return GDK_FILTER_CONTINUE;
}
}  // namespace Kiran