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
 * Author:     meizhigang <meizhigang@kylinsec.com.cn>
 */

#include "plugins/inputdevices/keyboard/modifier-lock-manager.h"

#include "lib/base/base.h"

#include <X11/XKBlib.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>

namespace Kiran
{
ModifierLockManager::ModifierLockManager(KeyboardManager *keyboard_manager) : keyboard_manager_(keyboard_manager)
{
}

ModifierLockManager::~ModifierLockManager()
{
    gdk_window_remove_filter(NULL,
                             &ModifierLockManager::window_event,
                             this);
}

ModifierLockManager *ModifierLockManager::instance_ = nullptr;

void ModifierLockManager::global_init(KeyboardManager *keyboard_manager)
{
    instance_ = new ModifierLockManager(keyboard_manager);

    if (keyboard_manager->is_modifier_lock_enabled())
    {
        instance_->init();
    }
}

void ModifierLockManager::init()
{
    Display *dpy = GDK_DISPLAY_XDISPLAY(gdk_display_get_default());

    this->capslock_mask_ = XkbKeysymToModifiers(dpy, XK_Caps_Lock);
    this->numlock_mask_ = XkbKeysymToModifiers(dpy, XK_Num_Lock);
    this->capslock_keycode_ = XKeysymToKeycode(dpy, XK_Caps_Lock);
    this->numlock_keycode_ = XKeysymToKeycode(dpy, XK_Num_Lock);

    KLOG_DEBUG("Xkb keysym capslock mask:%d, keycode:%d; numlock mask:%d, keycode:%d.",
               this->capslock_mask_, this->capslock_keycode_,
               this->numlock_mask_, this->numlock_keycode_);

    this->xkb_init();

    gdk_window_add_filter(NULL,
                          &ModifierLockManager::window_event,
                          this);

    return;
}

int ModifierLockManager::xkb_init()
{
    Display *dpy = GDK_DISPLAY_XDISPLAY(gdk_display_get_default());

    int opcode, error_base, major, minor;
    if (!XkbQueryExtension(dpy,
                           &opcode,
                           &this->xkb_event_base_,
                           &error_base,
                           &major,
                           &minor))
    {
        KLOG_ERROR("Query extension failed.");
        return CCErrorCode::ERROR_FAILED;
    }

    if (!XkbUseExtension(dpy, &major, &minor))
    {
        KLOG_ERROR("Use extension failed.");
        return CCErrorCode::ERROR_FAILED;
    }

    if (!XkbSelectEventDetails(dpy,
                               XkbUseCoreKbd,
                               XkbStateNotifyMask,
                               XkbModifierLockMask,
                               XkbModifierLockMask))
    {
        KLOG_ERROR("Select event details failed.");
        return CCErrorCode::ERROR_FAILED;
    }

    return CCErrorCode::SUCCESS;
}

GdkFilterReturn ModifierLockManager::window_event(GdkXEvent *gdk_event, GdkEvent *event, gpointer data)
{
    XEvent *xev = (XEvent *)gdk_event;
    ModifierLockManager *manager = (ModifierLockManager *)data;

    if (xev->type == manager->xkb_event_base_)
    {
        XkbEvent *xkbev = (XkbEvent *)xev;
        if ((xkbev->any.xkb_type == XkbStateNotify) && (xkbev->state.changed & XkbModifierLockMask))
        {
            manager->set_lock_action(xkbev->state.keycode, xkbev->state.locked_mods);
        }
    }

    return GDK_FILTER_CONTINUE;
}

void ModifierLockManager::set_lock_action(KeyCode keycode, unsigned int mods)
{
    KLOG_DEBUG("Choose action keycode:%d, mods:%d.", keycode, mods);

    if (keycode == this->capslock_keycode_)
    {
        RETURN_IF_FALSE(this->keyboard_manager_->is_capslock_tips_enabled());

        bool capslock_enable = !!(this->capslock_mask_ & mods);
        if (capslock_enable)
        {
            this->lock_window_.show_capslock_on();
        }
        else
        {
            this->lock_window_.show_capslock_off();
        }
    }
    else if (keycode == this->numlock_keycode_)
    {
        RETURN_IF_FALSE(this->keyboard_manager_->is_numlock_tips_enabled());

        bool numlock_enable = !!(this->numlock_mask_ & mods);
        if (numlock_enable)
        {
            this->lock_window_.show_numlock_on();
        }
        else
        {
            this->lock_window_.show_numlock_off();
        }
    }
    else
    {
        // others do nothing
    }

    return;
}

}  // namespace Kiran