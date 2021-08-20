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

CustomShortCutManager::CustomShortCutManager() : rand_((uint32_t)time(NULL))
{
    this->conf_file_path_ = Glib::build_filename(Glib::get_user_config_dir(),
                                                 KEYBINDING_CONF_DIR,
                                                 CUSTOM_SHORTCUT_FILE);
}

CustomShortCutManager::~CustomShortCutManager()
{
    if (this->root_window_)
    {
        this->root_window_->remove_filter(&CustomShortCutManager::window_event, this);
    }
}

CustomShortCutManager *CustomShortCutManager::instance_ = nullptr;
void CustomShortCutManager::global_init()
{
    instance_ = new CustomShortCutManager();
    instance_->init();
}

std::string CustomShortCutManager::add(std::shared_ptr<CustomShortCut> shortcut, CCErrorCode &error_code)
{
    KLOG_PROFILE("name: %s action: %s keycomb: %s.",
                 shortcut->name.c_str(),
                 shortcut->action.c_str(),
                 shortcut->key_combination.c_str());

    RETURN_VAL_IF_FALSE(this->check_valid(shortcut, error_code), std::string());

    auto tmp_uid = this->lookup_shortcut(shortcut->key_combination);
    if (tmp_uid != std::string())
    {
        error_code = CCErrorCode::ERROR_KEYBINDING_CUSTOM_KEYCOMB_ALREADY_EXIST_1;
        return std::string();
    }

    auto uid = this->gen_uid();
    if (uid.length() == 0)
    {
        KLOG_WARNING("cannot generate unique ID for custom shortcut.");
        error_code = CCErrorCode::ERROR_KEYBINDING_GEN_UID_FAILED;
        return std::string();
    }

    RETURN_VAL_IF_FALSE(this->grab_keycomb_change(shortcut->key_combination, true, error_code), std::string());

    this->change_and_save(uid, shortcut);

    return uid;
}

bool CustomShortCutManager::modify(const std::string &uid, std::shared_ptr<CustomShortCut> shortcut, CCErrorCode &error_code)
{
    KLOG_PROFILE("name: %s action: %s keycomb: %s.",
                 shortcut->name.c_str(),
                 shortcut->action.c_str(),
                 shortcut->key_combination.c_str());

    RETURN_VAL_IF_FALSE(this->check_valid(shortcut, error_code), false);

    if (!this->keyfile_.has_group(uid))
    {
        error_code = CCErrorCode::ERROR_KEYBINDING_CUSTOM_SHORTCUT_NOT_EXIST_1;
        return false;
    }

    auto tmp_uid = this->lookup_shortcut(shortcut->key_combination);
    if (tmp_uid != std::string() && tmp_uid != uid)
    {
        error_code = CCErrorCode::ERROR_KEYBINDING_CUSTOM_KEYCOMB_ALREADY_EXIST_2;
        return false;
    }

    auto old_key_comb = this->keyfile_.get_value(uid, CUSTOM_KEYFILE_KEYCOMB);

    if (old_key_comb != shortcut->key_combination)
    {
        RETURN_VAL_IF_FALSE(this->grab_keycomb_change(old_key_comb, false, error_code), false);
        RETURN_VAL_IF_FALSE(this->grab_keycomb_change(shortcut->key_combination, true, error_code), false);
    }

    this->change_and_save(uid, shortcut);
    return true;
}

bool CustomShortCutManager::remove(const std::string &uid, CCErrorCode &error_code)
{
    KLOG_PROFILE("id: %s.", uid.c_str());

    if (!this->keyfile_.has_group(uid))
    {
        error_code = CCErrorCode::ERROR_KEYBINDING_CUSTOM_SHORTCUT_NOT_EXIST_2;
        return false;
    }
    auto old_key_comb = this->keyfile_.get_value(uid, CUSTOM_KEYFILE_KEYCOMB);
    RETURN_VAL_IF_FALSE(this->grab_keycomb_change(old_key_comb, false, error_code), false);
    this->change_and_save(uid, nullptr);
    return true;
}

std::shared_ptr<CustomShortCut> CustomShortCutManager::get(const std::string &uid)
{
    KLOG_PROFILE("id: %s.", uid.c_str());
    if (!this->keyfile_.has_group(uid))
    {
        return nullptr;
    }
    auto shortcut = std::make_shared<CustomShortCut>();
    shortcut->name = this->keyfile_.get_value(uid, CUSTOM_KEYFILE_NAME);
    shortcut->action = this->keyfile_.get_value(uid, CUSTOM_KEYFILE_ACTION);
    shortcut->key_combination = this->keyfile_.get_value(uid, CUSTOM_KEYFILE_KEYCOMB);
    return shortcut;
}

std::map<std::string, std::shared_ptr<CustomShortCut>> CustomShortCutManager::get()
{
    KLOG_PROFILE("");
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

std::string CustomShortCutManager::lookup_shortcut(const std::string &keycomb)
{
    KLOG_PROFILE("keycomb: %s", keycomb.c_str());
    for (const auto &group : this->keyfile_.get_groups())
    {
        auto value = this->keyfile_.get_value(group, CUSTOM_KEYFILE_KEYCOMB);
        if (value == keycomb)
        {
            return group;
        }
    }
    return std::string();
}

void CustomShortCutManager::init()
{
    this->init_modifiers();
    try
    {
        this->keyfile_.load_from_file(this->conf_file_path_, Glib::KEY_FILE_KEEP_COMMENTS);
    }
    catch (const Glib::Error &e)
    {
        KLOG_WARNING("failed to load %s: %s.", this->conf_file_path_.c_str(), e.what().c_str());
    };

    auto display = Gdk::Display::get_default();
    this->root_window_ = display->get_default_screen()->get_root_window();
    this->root_window_->add_filter(&CustomShortCutManager::window_event, this);
    auto event_mask = this->root_window_->get_events();
    this->root_window_->set_events(event_mask | Gdk::KEY_PRESS_MASK);

    for (const auto &group : this->keyfile_.get_groups())
    {
        auto shortcut = this->get(group.raw());
        if (shortcut)
        {
            CCErrorCode error_code = CCErrorCode::SUCCESS;
            if (!this->check_valid(shortcut, error_code) ||
                !this->grab_keycomb_change(shortcut->key_combination, true, error_code))
            {
                shortcut->key_combination = SHORTCUT_KEYCOMB_DISABLE;
                this->change_and_save(group, shortcut);
                KLOG_WARNING("disable custom shortcut '%s'. error_code: %d.", shortcut->name.c_str(), int32_t(error_code));
            }
        }
    }
}

void CustomShortCutManager::init_modifiers()
{
    this->ignored_mods_ = GDK_LOCK_MASK | GDK_HYPER_MASK;
    this->used_mods_ = GDK_SHIFT_MASK | GDK_CONTROL_MASK | GDK_MOD1_MASK | GDK_MOD2_MASK | GDK_MOD3_MASK | GDK_MOD4_MASK |
                       GDK_MOD5_MASK | GDK_SUPER_MASK | GDK_META_MASK;

    auto numlock_modifier = this->get_numlock_modifier();
    this->ignored_mods_ |= numlock_modifier;
    this->used_mods_ &= ~numlock_modifier;
}

uint32_t CustomShortCutManager::get_numlock_modifier()
{
    uint32_t mask = 0;
    /*
    获取每个修饰键绑定的keycode列表，一共8个真实修饰键。
    xmodmap->max_keypermod：每个修饰键最多对应的keycode数
    xmodmap->modifiermap：修饰键与keycode对于关系，数组的[i*xmodmap->max_keypermod, i*xmodmap->max_keypermod+xmodmap->max_keypermod)位置的值为第i个修饰键对应的keycode
    */
    auto xmodmap = XGetModifierMapping(gdk_x11_get_default_xdisplay());
    auto map_size = 8 * xmodmap->max_keypermod;

    // NumLock修饰键为Mod1-Mod5修饰键中的一个，其定义为：如果一个keycode与XK_Num_Lock存在映射关系且与Mod1-Mod5修饰键中任意一个有映射，则这个keycode为NumLock修饰键
    // 下面遍历Mod1-Mod5修饰键
    for (int32_t i = 3 * xmodmap->max_keypermod; i < map_size; ++i)
    {
        int keycode = xmodmap->modifiermap[i];
        GdkKeymapKey *keys = NULL;
        guint *keyvals = NULL;
        int n_entries = 0;

        gdk_keymap_get_entries_for_keycode(Gdk::Display::get_default()->get_keymap(),
                                           keycode,
                                           &keys,
                                           &keyvals,
                                           &n_entries);

        for (int j = 0; j < n_entries; ++j)
        {
            if (keyvals[j] == GDK_KEY_Num_Lock)
            {
                mask |= (1 << (i / xmodmap->max_keypermod));
                break;
            }
        }

        g_free(keyvals);
        g_free(keys);
    }

    XFreeModifiermap(xmodmap);
    return mask;
}

std::string CustomShortCutManager::gen_uid()
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

bool CustomShortCutManager::check_valid(std::shared_ptr<CustomShortCut> shortcut, CCErrorCode &error_code)
{
    if (shortcut->name.length() == 0 ||
        shortcut->action.length() == 0)
    {
        KLOG_WARNING("the name or action is null string");
        error_code = CCErrorCode::ERROR_KEYBINDING_CUSTOM_SHORTCUT_INVALID;
        return false;
    }

    if (ShortCutHelper::get_keystate(shortcut->key_combination) == INVALID_KEYSTATE)
    {
        KLOG_WARNING("the format of the key_combination '%s' is invalid.", shortcut->key_combination.c_str());
        error_code = CCErrorCode::ERROR_KEYBINDING_CUSTOM_SHORTCUT_INVALID;
        return false;
    }
    return true;
}

void CustomShortCutManager::change_and_save(const std::string &uid, std::shared_ptr<CustomShortCut> shortcut)
{
    KLOG_PROFILE("uid: %s.", uid.c_str());

    if (shortcut)
    {
        this->keyfile_.set_value(uid, CUSTOM_KEYFILE_NAME, shortcut->name);
        this->keyfile_.set_value(uid, CUSTOM_KEYFILE_ACTION, shortcut->action);
        this->keyfile_.set_value(uid, CUSTOM_KEYFILE_KEYCOMB, shortcut->key_combination);
    }
    else
    {
        this->keyfile_.remove_group(uid);
    }

    RETURN_IF_TRUE(this->save_id_);
    auto timeout = Glib::MainContext::get_default()->signal_timeout();
    this->save_id_ = timeout.connect(sigc::mem_fun(this, &CustomShortCutManager::save_to_file), SAVE_TIMEOUT_MILLISECONDS);
}

bool CustomShortCutManager::save_to_file()
{
    this->keyfile_.save_to_file(this->conf_file_path_);
    return false;
}

bool CustomShortCutManager::grab_keycomb_change(const std::string &key_comb, bool grab, CCErrorCode &error_code)
{
    KLOG_PROFILE("key_comb: %s grab: %d.", key_comb.c_str(), grab);

    auto key_state = ShortCutHelper::get_keystate(key_comb);
    if (key_state == INVALID_KEYSTATE)
    {
        error_code = CCErrorCode::ERROR_KEYBINDING_CUSTOM_KEYCOMB_INVALID_1;
        return false;
    }
    return this->grab_keystate_change(key_state, grab, error_code);
}

bool CustomShortCutManager::grab_keystate_change(const KeyState &keystate, bool grab, CCErrorCode &error_code)
{
    KLOG_PROFILE("symbol: %0x mods: %0x", keystate.key_symbol, keystate.mods);

    RETURN_VAL_IF_TRUE(keystate == NULL_KEYSTATE, true);

    if (keystate == INVALID_KEYSTATE)
    {
        error_code = CCErrorCode::ERROR_KEYBINDING_CUSTOM_KEYCOMB_INVALID_2;
        return false;
    }

    std::vector<int32_t> mask_bits;
    uint32_t mask = this->ignored_mods_ & ~(keystate.mods) & GDK_MODIFIER_MASK;

    for (int32_t i = 0; mask; ++i, mask >>= 1)
    {
        if (mask & 0x1)
        {
            mask_bits.push_back(i);
        }
    }

    int32_t mask_state_num = (1 << mask_bits.size());

    for (int32_t i = 0; i < mask_state_num; ++i)
    {
        int32_t ignored_state_comb = 0;
        for (int32_t j = 0; j < (int32_t)mask_bits.size(); ++j)
        {
            if (i & (1 << j))
            {
                ignored_state_comb |= (1 << mask_bits[j]);
            }
        }

        auto display = gdk_display_get_default();
        gdk_x11_display_error_trap_push(display);

        for (auto &keycode : keystate.keycodes)
        {
            if (grab)
            {
                XGrabKey(GDK_DISPLAY_XDISPLAY(display),
                         keycode,
                         ignored_state_comb | keystate.mods,
                         GDK_WINDOW_XID(this->root_window_->gobj()),
                         True,
                         GrabModeAsync,
                         GrabModeAsync);
            }
            else
            {
                XUngrabKey(GDK_DISPLAY_XDISPLAY(display),
                           keycode,
                           ignored_state_comb | keystate.mods,
                           GDK_WINDOW_XID(this->root_window_->gobj()));
            }
        }
        if (gdk_x11_display_error_trap_pop(display))
        {
            KLOG_WARNING("Grab failed for some keys, another application may already have access the them.");
            error_code = CCErrorCode::ERROR_KEYBINDING_GRAB_KEY_FAILED;
            return false;
        }
    }
    return true;
}

GdkFilterReturn CustomShortCutManager::window_event(GdkXEvent *gdk_event, GdkEvent *event, gpointer data)
{
    XEvent *xevent = (XEvent *)gdk_event;
    RETURN_VAL_IF_TRUE(xevent->type != KeyPress, GDK_FILTER_CONTINUE);

    CustomShortCutManager *manager = (CustomShortCutManager *)data;
    g_return_val_if_fail(CustomShortCutManager::get_instance() == manager, GDK_FILTER_REMOVE);

    for (const auto &group : manager->keyfile_.get_groups())
    {
        auto name = manager->keyfile_.get_value(group, CUSTOM_KEYFILE_NAME);
        auto action = manager->keyfile_.get_value(group, CUSTOM_KEYFILE_ACTION);
        auto key_combination = manager->keyfile_.get_value(group, CUSTOM_KEYFILE_KEYCOMB);

        auto key_state = ShortCutHelper::get_keystate(key_combination);
        auto event_key_state = ShortCutHelper::get_keystate(xevent);
        KLOG_DEBUG("key_comb: %s key_state: %0x %0x event_key_state: %0x %0x %0x.",
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
                KLOG_WARNING("failed to parse %s: %s.", action.c_str(), e.what().c_str());
                return GDK_FILTER_CONTINUE;
            }

            try
            {
                Glib::spawn_async(std::string(), argv, Glib::SPAWN_SEARCH_PATH);
            }
            catch (const Glib::Error &e)
            {
                KLOG_WARNING("failed to exec %s: %s.", action.c_str(), e.what().c_str());
            }

            return GDK_FILTER_REMOVE;
        }
    }

    return GDK_FILTER_CONTINUE;
}

}  // namespace Kiran