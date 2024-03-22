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

#include "plugins/power/event/power-event-button.h"

#include <X11/XF86keysym.h>

namespace Kiran
{
#define POWER_BUTTON_DUPLICATE_TIMEOUT 0.125f

PowerEventButton::PowerEventButton() : login1_inhibit_fd_(-1)
{
    this->display_ = gdk_display_get_default();
    this->xdisplay_ = GDK_DISPLAY_XDISPLAY(this->display_);

    auto screen = gdk_screen_get_default();
    this->root_window_ = gdk_screen_get_root_window(screen);
    this->xroot_window_ = GDK_WINDOW_XID(this->root_window_);

    this->upower_client_ = PowerWrapperManager::get_instance()->get_default_upower();
}

PowerEventButton::~PowerEventButton()
{
    gdk_window_remove_filter(this->root_window_, &PowerEventButton::window_event, this);

    if (this->login1_inhibit_fd_ > 0)
    {
        close(this->login1_inhibit_fd_);
    }
}

void PowerEventButton::init()
{
    // 这里需要对systemd-login1添加抑制器，避免systemd-login1对电源、休眠、挂起按键和合上盖子进行操作。
    auto login1 = PowerWrapperManager::get_instance()->get_default_login1();
    this->login1_inhibit_fd_ = login1->inhibit("handle-power-key:handle-suspend-key:handle-lid-switch");

    this->register_button(XF86XK_PowerOff, PowerEvent::POWER_EVENT_PRESSED_POWEROFF);
    this->register_button(XF86XK_Suspend, PowerEvent::POWER_EVENT_PRESSED_SUSPEND);
    this->register_button(XF86XK_Sleep, PowerEvent::POWER_EVENT_PRESSED_SLEEP);
    this->register_button(XF86XK_Hibernate, PowerEvent::POWER_EVENT_PRESSED_HIBERNATE);
    this->register_button(XF86XK_MonBrightnessUp, PowerEvent::POWER_EVENT_PRESSED_BRIGHT_UP);
    this->register_button(XF86XK_MonBrightnessDown, PowerEvent::POWER_EVENT_PRESSED_BRIGHT_DOWN);
    this->register_button(XF86XK_KbdBrightnessUp, PowerEvent::POWER_EVENT_PRESSED_KBD_BRIGHT_UP);
    this->register_button(XF86XK_KbdBrightnessDown, PowerEvent::POWER_EVENT_PRESSED_KBD_BRIGHT_DOWN);
    this->register_button(XF86XK_KbdLightOnOff, PowerEvent::POWER_EVENT_PRESSED_KBD_BRIGHT_TOGGLE);
    this->register_button(XF86XK_ScreenSaver, PowerEvent::POWER_EVENT_PRESSED_LOCK);
    this->register_button(XF86XK_Battery, PowerEvent::POWER_EVENT_PRESSED_BATTERY);

    this->upower_client_->signal_lid_is_closed_changed().connect(sigc::mem_fun(this, &PowerEventButton::on_lid_is_closed_change));

    gdk_window_add_filter(this->root_window_, &PowerEventButton::window_event, this);
}

bool PowerEventButton::register_button(uint32_t keysym, PowerEvent type)
{
    auto keycode = XKeysymToKeycode(this->xdisplay_, keysym);
    if (keycode == 0)
    {
        KLOG_WARNING("Could not map keysym 0x%x to keycode", keysym);
        return false;
    }

    KLOG_DEBUG("keysym: 0x%08x, keycode: 0x%08x.", keysym, keycode);

    auto keycode_str = fmt::format("0x{:x}", keycode);

    auto iter = this->buttons_.emplace(keycode_str, type);
    if (!iter.second)
    {
        KLOG_WARNING("Already exists keycode: %s.", keycode_str.c_str());
        return false;
    }

    gdk_x11_display_error_trap_push(this->display_);

    auto ret = XGrabKey(this->xdisplay_,
                        keycode,
                        AnyModifier,
                        this->xroot_window_,
                        True,
                        GrabModeAsync,
                        GrabModeAsync);
    if (ret == BadAccess)
    {
        KLOG_WARNING("Failed to grab keycode: %d", (int32_t)keycode);
        return false;
    }

    gdk_display_flush(this->display_);
    gdk_x11_display_error_trap_pop_ignored(this->display_);

    return true;
}

void PowerEventButton::emit_button_signal(PowerEvent type)
{
    unsigned long elapsed_msec;
    if (this->button_signal_timer_.elapsed(elapsed_msec) < POWER_BUTTON_DUPLICATE_TIMEOUT)
    {
        KLOG_DEBUG("ignoring duplicate button %d", type);
        return;
    }

    this->button_changed_.emit(type);
    this->button_signal_timer_.reset();
}

void PowerEventButton::on_lid_is_closed_change(bool lid_is_closed)
{
    if (lid_is_closed)
    {
        this->button_changed_.emit(PowerEvent::POWER_EVENT_LID_CLOSED);
    }
    else
    {
        this->button_changed_.emit(PowerEvent::POWER_EVENT_LID_OPEN);
    }
}

GdkFilterReturn PowerEventButton::window_event(GdkXEvent *gdk_event, GdkEvent *event, gpointer data)
{
    auto button = (PowerEventButton *)data;
    XEvent *xevent = (XEvent *)gdk_event;

    RETURN_VAL_IF_TRUE(xevent->type != KeyPress, GDK_FILTER_CONTINUE);

    auto keycode = xevent->xkey.keycode;
    auto keycode_str = fmt::format("0x{:x}", keycode);

    auto iter = button->buttons_.find(keycode_str);
    if (iter == button->buttons_.end())
    {
        KLOG_DEBUG("Keycode %d not found.", keycode);
        return GDK_FILTER_CONTINUE;
    }

    KLOG_DEBUG("Receipt keycode signal: %s.", keycode_str.c_str());
    button->emit_button_signal(iter->second);

    return GDK_FILTER_REMOVE;
}

}  // namespace Kiran