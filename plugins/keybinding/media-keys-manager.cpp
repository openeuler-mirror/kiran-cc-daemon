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

#include "plugins/keybinding/media-keys-manager.h"
#include <gdk/gdkkeysyms.h>
#include <gdk/gdkx.h>

#include "media-keys-i.h"

namespace Kiran
{
MediaKeysManager::MediaKeysManager(KeybindingManager *keybinding_manager) : super_valid_flag_(false)
{
    this->actions_ = std::make_shared<MediaKeysAction>();
    this->system_shortcuts_ = keybinding_manager->get_system_shortcuts();
}

MediaKeysManager::~MediaKeysManager()
{
}

MediaKeysManager *MediaKeysManager::instance_ = nullptr;
void MediaKeysManager::global_init(KeybindingManager *keybinding_manager)
{
    instance_ = new MediaKeysManager(keybinding_manager);
    instance_->init();
}

void MediaKeysManager::init()
{
    this->actions_->init();

    this->init_modifiers();

    this->system_shortcuts_->signal_shortcut_added().connect(sigc::mem_fun(this, &MediaKeysManager::system_shortcut_added));
    this->system_shortcuts_->signal_shortcut_deleted().connect(sigc::mem_fun(this, &MediaKeysManager::system_shortcut_deleted));
    this->system_shortcuts_->signal_shortcut_changed().connect(sigc::mem_fun(this, &MediaKeysManager::system_shortcut_changed));

    auto display = Gdk::Display::get_default();
    this->root_window_ = display->get_default_screen()->get_root_window();
    this->root_window_->add_filter(&MediaKeysManager::window_event, this);
    auto event_mask = this->root_window_->get_events();
    this->root_window_->set_events(event_mask | Gdk::KEY_PRESS_MASK | Gdk::KEY_RELEASE_MASK);

    this->init_grab_keys();
}

void MediaKeysManager::init_modifiers()
{
    this->ignored_mods_ = 0x2000 | GDK_LOCK_MASK | GDK_HYPER_MASK;
    this->used_mods_ = GDK_SHIFT_MASK | GDK_CONTROL_MASK | GDK_MOD1_MASK | GDK_MOD2_MASK | GDK_MOD3_MASK | GDK_MOD4_MASK |
                       GDK_MOD5_MASK | GDK_SUPER_MASK | GDK_META_MASK;

    int ignord_mod_mask = ShortCutHelper::get_numlock_modifier();

    this->ignored_mods_ |= ignord_mod_mask;
    this->used_mods_ &= ~ignord_mod_mask;
}

void MediaKeysManager::init_grab_keys()
{
    auto system_shortcuts = this->system_shortcuts_->get();

    for (auto &shortcut : system_shortcuts)
    {
        if (shortcut.second->settings->property_schema_id() != MEDIAKEYS_SCHEMA_ID)
        {
            continue;
        }

        KeyState key_state = ShortCutHelper::get_keystate(shortcut.second->key_combination);

        if (key_state == INVALID_KEYSTATE)
        {
            KLOG_WARNING_KEYBINDING("Invalid key state  comb:%s.", shortcut.second->key_combination.c_str());
            continue;
        }

        bool grab_retval = ShortCutHelper::grab_keystate_change(this->root_window_, this->ignored_mods_, key_state, true);
        if (!grab_retval)
        {
            KLOG_WARNING_KEYBINDING("Grab key state failed comb:%s.", shortcut.second->key_combination.c_str());
            continue;
        }

        std::shared_ptr<MediaKeysShortCut> mediakeys_shortcut = std::make_shared<MediaKeysShortCut>();
        mediakeys_shortcut->uid = shortcut.second->uid;
        mediakeys_shortcut->key_combination = shortcut.second->key_combination;
        mediakeys_shortcut->settings_key = shortcut.second->settings_key;

        auto iter = this->shortcuts_.emplace(mediakeys_shortcut->uid, mediakeys_shortcut);
        if (!iter.second)
        {
            KLOG_WARNING_KEYBINDING("Exists the same system shortcut, uid: %s ",
                                    mediakeys_shortcut->uid.c_str());
            continue;
        }

        KLOG_DEBUG_KEYBINDING("Grab key state comb:%s, symbol:%u, mods:0x%0x.",
                              mediakeys_shortcut->key_combination.c_str(),
                              key_state.key_symbol, key_state.mods);
    }
}

bool MediaKeysManager::is_valid_key_event(XEvent *xev)
{
    if (xev->type != KeyPress && xev->type != KeyRelease)
    {
        return false;
    }

    Display *dpy = gdk_x11_get_default_xdisplay();
    if (XKeysymToKeycode(dpy, XK_Super_L) == xev->xkey.keycode)
    {
        if (xev->type == KeyPress)
        {
            this->super_valid_flag_ = true;
            return false;
        }
        else
        {
            // if falg is Ture then the super release is valid
            return this->super_valid_flag_;
        }
    }
    else
    {
        this->super_valid_flag_ = false;
        if (xev->type == KeyRelease)
        {
            return false;
        }
    }

    return true;
}

GdkFilterReturn MediaKeysManager::window_event(GdkXEvent *gdk_event, GdkEvent *event, gpointer data)
{
    XEvent *xevent = static_cast<XEvent *>(gdk_event);
    MediaKeysManager *manager = static_cast<MediaKeysManager *>(data);

    RETURN_VAL_IF_FALSE(manager->is_valid_key_event(xevent), GDK_FILTER_CONTINUE);

    auto event_key_state = ShortCutHelper::get_keystate(xevent);

    KLOG_DEBUG_KEYBINDING("Media window keycode:0x%0x. symbol:0x%0x, mods:0x%0x.",
                          xevent->xkey.keycode,
                          event_key_state.key_symbol,
                          event_key_state.mods);

    for (const auto &shortcut : manager->shortcuts_)
    {
        auto key_state = ShortCutHelper::get_keystate(shortcut.second->key_combination);

        // Super键作为真实按键抓取，返回按键mods带有修饰键信息，判断Super按键时去掉mods
        if ((key_state.key_symbol == event_key_state.key_symbol) &&
            (manager->super_valid_flag_ ? true : (key_state.mods == (event_key_state.mods & manager->used_mods_))))
        {
            if (manager->actions_->do_action(xevent, shortcut.second->settings_key))
            {
                return GDK_FILTER_REMOVE;
            }
            else
            {
                return GDK_FILTER_CONTINUE;
            }
        }
    }

    return GDK_FILTER_CONTINUE;
}

void MediaKeysManager::system_shortcut_added(std::shared_ptr<SystemShortCut> system_shortcut)
{
    auto iter = this->shortcuts_.find(system_shortcut->uid);
    if (iter != this->shortcuts_.end())
    {
        KLOG_WARNING_KEYBINDING("The shortcut %s is exists.", system_shortcut->uid.c_str());
        return;
    }

    auto key_state = ShortCutHelper::get_keystate(system_shortcut->key_combination);
    if (key_state == INVALID_KEYSTATE)
    {
        KLOG_WARNING_KEYBINDING("Invalid key state key:%s, comb:%s.", system_shortcut->settings_key.c_str(), system_shortcut->key_combination.c_str());
        return;
    }

    auto grab_retval = ShortCutHelper::grab_keystate_change(this->root_window_, this->ignored_mods_, key_state, true);
    if (!grab_retval)
    {
        KLOG_WARNING_KEYBINDING("Grab key state failed comb:%s.", system_shortcut->key_combination.c_str());
        return;
    }

    std::shared_ptr<MediaKeysShortCut> mediakeys_shortcut = std::make_shared<MediaKeysShortCut>();
    mediakeys_shortcut->uid = system_shortcut->uid;
    mediakeys_shortcut->key_combination = system_shortcut->key_combination;
    mediakeys_shortcut->settings_key = system_shortcut->settings_key;

    auto iter_new = this->shortcuts_.emplace(mediakeys_shortcut->uid, mediakeys_shortcut);
    if (!iter_new.second)
    {
        KLOG_WARNING_KEYBINDING("Exists the same system shortcut, uid: %s ",
                                mediakeys_shortcut->uid.c_str());
    }
}

void MediaKeysManager::system_shortcut_deleted(std::shared_ptr<SystemShortCut> system_shortcut)
{
    auto iter = this->shortcuts_.find(system_shortcut->uid);
    if (iter == this->shortcuts_.end())
    {
        KLOG_WARNING_KEYBINDING("The shortcut %s is not exists.", system_shortcut->uid.c_str());
        return;
    }

    auto key_state = ShortCutHelper::get_keystate(system_shortcut->key_combination);
    if (key_state == INVALID_KEYSTATE)
    {
        KLOG_WARNING_KEYBINDING("Invalid key state key:%s, comb:%s.", system_shortcut->settings_key.c_str(), system_shortcut->key_combination.c_str());
        return;
    }

    bool grab_retval = ShortCutHelper::grab_keystate_change(this->root_window_, this->ignored_mods_, key_state, false);
    if (!grab_retval)
    {
        KLOG_WARNING_KEYBINDING("Grab key state failed comb:%s.", system_shortcut->key_combination.c_str());
        return;
    }

    this->shortcuts_.erase(iter);
}

void MediaKeysManager::system_shortcut_changed(std::shared_ptr<SystemShortCut> system_shortcut)
{
    auto iter = this->shortcuts_.find(system_shortcut->uid);
    if (iter == this->shortcuts_.end())
    {
        KLOG_WARNING_KEYBINDING("The shortcut %s is not exists.", system_shortcut->uid.c_str());
        return;
    }

    auto old_key_comb = iter->second->key_combination;
    if (old_key_comb != system_shortcut->key_combination)
    {
        KLOG_DEBUG_KEYBINDING("The system_shortcut_changed  old_key_comb%s,key_comb:%s",
                              old_key_comb.c_str(), system_shortcut->key_combination.c_str());

        auto old_key_state = ShortCutHelper::get_keystate(old_key_comb);
        if (old_key_state == INVALID_KEYSTATE)
        {
            KLOG_WARNING_KEYBINDING("Invalid key state key:%s, comb:%s.", system_shortcut->settings_key.c_str(), system_shortcut->key_combination.c_str());
            return;
        }

        bool grab_retval = ShortCutHelper::grab_keystate_change(this->root_window_, this->ignored_mods_, old_key_state, false);
        if (!grab_retval)
        {
            KLOG_WARNING_KEYBINDING("Grab key state failed comb:%s.", system_shortcut->key_combination.c_str());
            return;
        }

        auto key_state = ShortCutHelper::get_keystate(system_shortcut->key_combination);
        if (key_state == INVALID_KEYSTATE)
        {
            KLOG_WARNING_KEYBINDING("Invalid key state key:%s, comb:%s.", system_shortcut->settings_key.c_str(), system_shortcut->key_combination.c_str());
            return;
        }

        grab_retval = ShortCutHelper::grab_keystate_change(this->root_window_, this->ignored_mods_, key_state, true);
        if (!grab_retval)
        {
            KLOG_WARNING_KEYBINDING("Grab key state failed comb:%s.", system_shortcut->key_combination.c_str());
            return;
        }

        iter->second->key_combination = system_shortcut->key_combination;
    }
}

}  // namespace Kiran
