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

#include "plugins/keybinding/custom-shortcut.h"

#include <gdk/gdkkeysyms.h>
#include <gdk/gdkx.h>

#include "plugins/keybinding/shortcut-helper.h"

namespace Kiran
{
#define CUSTOM_SHORTCUT_FILE "custom_shortcut.ini"
#define CUSTOM_KEYFILE_NAME "name"
#define CUSTOM_KEYFILE_ACTION "action"
#define CUSTOM_KEYFILE_KEYCOMB "key_combination"
#define GEN_ID_MAX_NUM 5
#define SAVE_TIMEOUT_MILLISECONDS 500

CustomShortCuts::CustomShortCuts() : rand_((uint32_t)time(NULL))
{
    this->conf_file_path_ = Glib::build_filename(Glib::get_user_config_dir(),
                                                 KEYBINDING_CONF_DIR,
                                                 CUSTOM_SHORTCUT_FILE);
}

CustomShortCuts::~CustomShortCuts()
{
    if (this->root_window_)
    {
        this->root_window_->remove_filter(&CustomShortCuts::window_event, this);
    }
}

void CustomShortCuts::init()
{
    this->init_modifiers();
    try
    {
        this->keyfile_.load_from_file(this->conf_file_path_, Glib::KEY_FILE_KEEP_COMMENTS);
    }
    catch (const Glib::Error &e)
    {
        KLOG_WARNING_KEYBINDING("Failed to load %s: %s.", this->conf_file_path_.c_str(), e.what().c_str());
    };

    auto display = Gdk::Display::get_default();
    this->root_window_ = display->get_default_screen()->get_root_window();
    this->root_window_->add_filter(&CustomShortCuts::window_event, this);
    auto event_mask = this->root_window_->get_events();
    this->root_window_->set_events(event_mask | Gdk::KEY_PRESS_MASK);

    for (const auto &group : this->keyfile_.get_groups())
    {
        auto shortcut = this->get(group.raw());
        if (shortcut)
        {
            if (!this->check_valid(shortcut) ||
                !this->grab_keycomb_change(shortcut->key_combination, true))
            {
                shortcut->key_combination = SHORTCUT_KEYCOMB_DISABLE;
                this->change_and_save(shortcut);
                KLOG_WARNING_KEYBINDING("Disable custom shortcut %s.", shortcut->name.c_str());
            }
        }
    }
}

bool CustomShortCuts::add(std::shared_ptr<CustomShortCut> shortcut)
{
    KLOG_DEBUG_KEYBINDING("Add custom shortcut key,Name: %s action: %s keycomb: %s.",
                          shortcut->name.c_str(),
                          shortcut->action.c_str(),
                          shortcut->key_combination.c_str());

    RETURN_VAL_IF_FALSE(this->check_valid(shortcut), false);

    auto uid = this->gen_uid();
    if (uid.length() == 0)
    {
        KLOG_WARNING_KEYBINDING("Cannot generate unique ID for custom shortcut.");
        return false;
    }

    RETURN_VAL_IF_FALSE(this->grab_keycomb_change(shortcut->key_combination, true), false);

    shortcut->uid = uid;
    this->change_and_save(shortcut, false);
    return true;
}

bool CustomShortCuts::modify(std::shared_ptr<CustomShortCut> shortcut)
{
    KLOG_DEBUG_KEYBINDING("Modify custom shortcut key,Name is %s,action is %s,keycomb is %s.",
                          shortcut->name.c_str(),
                          shortcut->action.c_str(),
                          shortcut->key_combination.c_str());

    RETURN_VAL_IF_FALSE(this->check_valid(shortcut), false);

    if (!this->keyfile_.has_group(shortcut->uid))
    {
        KLOG_WARNING_KEYBINDING("The shortcut %s is not exists.", shortcut->uid.c_str());
        return false;
    }

    auto old_key_comb = this->keyfile_.get_value(shortcut->uid, CUSTOM_KEYFILE_KEYCOMB);

    if (old_key_comb != shortcut->key_combination)
    {
        RETURN_VAL_IF_FALSE(this->grab_keycomb_change(old_key_comb, false), false);
        RETURN_VAL_IF_FALSE(this->grab_keycomb_change(shortcut->key_combination, true), false);
    }

    this->change_and_save(shortcut, false);
    return true;
}

bool CustomShortCuts::remove(const std::string &uid)
{
    KLOG_DEBUG_KEYBINDING("Remove custom shortcut key by Uid: %s.", uid.c_str());

    auto shortcut = this->get(uid);
    if (!shortcut)
    {
        KLOG_WARNING_KEYBINDING("The keycomb %s is not exists.", uid.c_str());
        return false;
    }

    RETURN_VAL_IF_FALSE(this->grab_keycomb_change(shortcut->key_combination, false), false);
    this->change_and_save(shortcut, true);
    return true;
}

std::shared_ptr<CustomShortCut> CustomShortCuts::get(const std::string &uid)
{
    if (!this->keyfile_.has_group(uid))
    {
        return nullptr;
    }
    auto shortcut = std::make_shared<CustomShortCut>();
    shortcut->uid = uid;
    shortcut->name = this->keyfile_.get_value(uid, CUSTOM_KEYFILE_NAME);
    shortcut->action = this->keyfile_.get_value(uid, CUSTOM_KEYFILE_ACTION);
    shortcut->key_combination = this->keyfile_.get_value(uid, CUSTOM_KEYFILE_KEYCOMB);
    return shortcut;
}

std::shared_ptr<CustomShortCut> CustomShortCuts::get_by_keycomb(const std::string &keycomb)
{
    KLOG_DEBUG_KEYBINDING("Get custom shortcut key by Keycomb: %s", keycomb.c_str());

    // 禁用快捷键不进行搜索
    RETURN_VAL_IF_TRUE(keycomb == SHORTCUT_KEYCOMB_DISABLE, nullptr);

    for (const auto &group : this->keyfile_.get_groups())
    {
        auto value = this->keyfile_.get_value(group, CUSTOM_KEYFILE_KEYCOMB);
        if (value == keycomb)
        {
            return this->get(group);
        }
    }
    return nullptr;
}

std::map<std::string, std::shared_ptr<CustomShortCut>> CustomShortCuts::get()
{
    std::map<std::string, std::shared_ptr<CustomShortCut>> shortcuts;
    for (const auto &group : this->keyfile_.get_groups())
    {
        auto shortcut = this->get(group);
        if (shortcut)
        {
            shortcuts.emplace(group, shortcut);
        }
    }
    return shortcuts;
}

void CustomShortCuts::init_modifiers()
{
    this->ignored_mods_ = GDK_LOCK_MASK | GDK_HYPER_MASK;
    this->used_mods_ = GDK_SHIFT_MASK | GDK_CONTROL_MASK | GDK_MOD1_MASK | GDK_MOD2_MASK | GDK_MOD3_MASK | GDK_MOD4_MASK |
                       GDK_MOD5_MASK | GDK_SUPER_MASK | GDK_META_MASK;

    auto numlock_modifier = ShortCutHelper::get_numlock_modifier();
    this->ignored_mods_ |= numlock_modifier;
    this->used_mods_ &= ~numlock_modifier;
}

std::string CustomShortCuts::gen_uid()
{
    for (int i = 0; i < GEN_ID_MAX_NUM; ++i)
    {
        auto rand_str = fmt::format("Custom{0}", this->rand_.get_int());
        if (!this->keyfile_.has_group(rand_str))
        {
            return rand_str;
        }
    }
    return std::string();
}

bool CustomShortCuts::check_valid(std::shared_ptr<CustomShortCut> shortcut)
{
    if (shortcut->name.length() == 0 ||
        shortcut->action.length() == 0)
    {
        KLOG_WARNING_KEYBINDING("The name or action is null string");
        return false;
    }

    if (ShortCutHelper::get_keystate(shortcut->key_combination) == INVALID_KEYSTATE)
    {
        KLOG_WARNING_KEYBINDING("The format of the key_combination '%s' is invalid.", shortcut->key_combination.c_str());
        return false;
    }
    return true;
}

void CustomShortCuts::change_and_save(std::shared_ptr<CustomShortCut> shortcut, bool is_remove)
{
    RETURN_IF_FALSE(shortcut);
    RETURN_IF_FALSE(!(shortcut->uid.empty()));

    if (!is_remove)
    {
        this->keyfile_.set_value(shortcut->uid, CUSTOM_KEYFILE_NAME, shortcut->name);
        this->keyfile_.set_value(shortcut->uid, CUSTOM_KEYFILE_ACTION, shortcut->action);
        this->keyfile_.set_value(shortcut->uid, CUSTOM_KEYFILE_KEYCOMB, shortcut->key_combination);
    }
    else
    {
        this->keyfile_.remove_group(shortcut->uid);
    }

    RETURN_IF_TRUE(this->save_id_);
    auto timeout = Glib::MainContext::get_default()->signal_timeout();
    this->save_id_ = timeout.connect(sigc::mem_fun(this, &CustomShortCuts::save_to_file), SAVE_TIMEOUT_MILLISECONDS);
}

bool CustomShortCuts::save_to_file()
{
    this->keyfile_.save_to_file(this->conf_file_path_);
    return false;
}

bool CustomShortCuts::grab_keycomb_change(const std::string &key_comb, bool is_grab)
{
    KLOG_DEBUG_KEYBINDING("The grab status of key_comb %s changed,and current grab status is %d.", key_comb.c_str(), is_grab);
    auto key_state = ShortCutHelper::get_keystate(key_comb);
    RETURN_VAL_IF_FALSE(key_state != INVALID_KEYSTATE, false);
    return ShortCutHelper::grab_keystate_change(this->root_window_, this->ignored_mods_, key_state, is_grab);
}

GdkFilterReturn CustomShortCuts::window_event(GdkXEvent *gdk_event, GdkEvent *event, gpointer data)
{
    XEvent *xevent = static_cast<XEvent *>(gdk_event);
    RETURN_VAL_IF_TRUE(xevent->type != KeyPress, GDK_FILTER_CONTINUE);

    CustomShortCuts *manager = static_cast<CustomShortCuts *>(data);

    for (const auto &group : manager->keyfile_.get_groups())
    {
        auto name = manager->keyfile_.get_value(group, CUSTOM_KEYFILE_NAME);
        auto action = manager->keyfile_.get_value(group, CUSTOM_KEYFILE_ACTION);
        auto key_combination = manager->keyfile_.get_value(group, CUSTOM_KEYFILE_KEYCOMB);

        auto key_state = ShortCutHelper::get_keystate(key_combination);
        auto event_key_state = ShortCutHelper::get_keystate(xevent);
        KLOG_DEBUG_KEYBINDING("Key_comb: %s key_state: %0x %0x event_key_state: %0x %0x %0x.",
                              key_combination.c_str(),
                              key_state.key_symbol, key_state.mods,
                              event_key_state.key_symbol, event_key_state.mods, event_key_state.mods & manager->used_mods_);
        if (key_state.key_symbol == event_key_state.key_symbol &&
            key_state.mods == (event_key_state.mods & manager->used_mods_))
        {
            std::vector<std::string> argv;
            try
            {
                argv = Glib::shell_parse_argv(action);
            }
            catch (const Glib::Error &e)
            {
                KLOG_WARNING_KEYBINDING("Failed to parse %s: %s.", action.c_str(), e.what().c_str());
                return GDK_FILTER_CONTINUE;
            }

            try
            {
                Glib::spawn_async(std::string(), argv, Glib::SPAWN_SEARCH_PATH);
            }
            catch (const Glib::Error &e)
            {
                KLOG_WARNING_KEYBINDING("Failed to exec %s: %s.", action.c_str(), e.what().c_str());
            }

            return GDK_FILTER_REMOVE;
        }
    }

    return GDK_FILTER_CONTINUE;
}

}  // namespace Kiran