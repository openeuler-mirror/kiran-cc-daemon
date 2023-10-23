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

#include "plugins/keybinding/shortcut-helper.h"

#include <X11/XKBlib.h>
#include <X11/extensions/XKB.h>
#include <gdk/gdkkeysyms.h>
#include <gdk/gdkx.h>

#include "lib/base/base.h"

namespace Kiran
{
KeyState ShortCutHelper::get_keystate(const std::string &key_comb)
{
    RETURN_VAL_IF_TRUE(key_comb.length() == 0, NULL_KEYSTATE);
    RETURN_VAL_IF_TRUE(StrUtils::tolower(key_comb) == SHORTCUT_KEYCOMB_DISABLE, NULL_KEYSTATE);

    KeyState key_state;
    size_t cur_pos = 0;

    while (cur_pos < key_comb.size())
    {
        if (key_comb[cur_pos] == '<')
        {
            auto gt_pos = key_comb.find('>', cur_pos);
            if (gt_pos == std::string::npos)
            {
                return INVALID_KEYSTATE;
            }
            auto token = StrUtils::tolower(key_comb.substr(cur_pos, gt_pos - cur_pos + 1));
            cur_pos = gt_pos + 1;
            switch (shash(token.c_str()))
            {
            case "<release>"_hash:
                key_state.mods |= GDK_RELEASE_MASK;
                break;
            case "<primary>"_hash:
            case "<control>"_hash:
            case "<ctrl>"_hash:
            case "<ctl>"_hash:
                key_state.mods |= GDK_CONTROL_MASK;
                break;
            case "<shift>"_hash:
            case "<shft>"_hash:
                key_state.mods |= GDK_SHIFT_MASK;
                break;
            case "<mod1>"_hash:
            case "<alt>"_hash:
                key_state.mods |= GDK_MOD1_MASK;
                break;
            case "<mod2>"_hash:
                key_state.mods |= GDK_MOD2_MASK;
                break;
            case "<mod3>"_hash:
                key_state.mods |= GDK_MOD3_MASK;
                break;
            case "<mod4>"_hash:
                key_state.mods |= GDK_MOD4_MASK;
                break;
            case "<mod5>"_hash:
                key_state.mods |= GDK_MOD5_MASK;
                break;
            case "<meta>"_hash:
                key_state.mods |= GDK_META_MASK;
                break;
            case "<hyper>"_hash:
                key_state.mods |= GDK_HYPER_MASK;
                break;
            case "<super>"_hash:
                key_state.mods |= GDK_SUPER_MASK;
                break;
            default:
                return INVALID_KEYSTATE;
            }
        }
        else
        {
            auto keyval = gdk_keyval_from_name(key_comb.substr(cur_pos).c_str());
            RETURN_VAL_IF_TRUE(keyval == GDK_KEY_VoidSymbol, INVALID_KEYSTATE);
            key_state.key_symbol = gdk_keyval_to_lower(keyval);
            key_state.keycodes = ShortCutHelper::get_keycode(key_state.key_symbol, [](int group, int level) -> bool
                                                             { return level == 0; });
            break;
        }
    }
    return key_state;
}

KeyState ShortCutHelper::get_keystate(XEvent *event)
{
    guint keyval;
    GdkModifierType consumed;
    KeyState key_state;

    auto group = XkbGroupForCoreState(event->xkey.state);

    /* Check if we find a keysym that matches our current state */
    if (gdk_keymap_translate_keyboard_state(gdk_keymap_get_for_display(gdk_display_get_default()),
                                            event->xkey.keycode,
                                            GdkModifierType(event->xkey.state),
                                            group,
                                            &keyval,
                                            NULL,
                                            NULL,
                                            &consumed))
    {
        guint lower, upper;

        gdk_keyval_convert_case(keyval, &lower, &upper);
        key_state.key_symbol = lower;
        KLOG_DEBUG_KEYBINDING("The keystate is %0x and consumed is %0x.", event->xkey.state, consumed);
        key_state.mods = event->xkey.state & ~consumed & GDK_MODIFIER_MASK;
        return key_state;
    }
    return INVALID_KEYSTATE;
}

std::vector<uint32_t> ShortCutHelper::get_keycode(uint32_t key_symbol, KeyCodeFilter filter)
{
    std::vector<uint32_t> result;
    GdkKeymapKey *keys;
    int32_t n_keys;
    if (gdk_keymap_get_entries_for_keyval(Gdk::Display::get_default()->get_keymap(),
                                          key_symbol,
                                          &keys,
                                          &n_keys))
    {
        for (int32_t i = 0; i < n_keys; ++i)
        {
            KLOG_DEBUG_KEYBINDING("%d keysym: %0x level: %d grouop: %d keycode: %0x.", i, key_symbol, keys[i].level, keys[i].group, keys[i].keycode);
            if (filter(keys[i].group, keys[i].level))
            {
                result.push_back(keys[i].keycode);
            }
        }
    }
    auto iter = std::unique(result.begin(), result.end());
    result.erase(iter, result.end());
    return result;
}
}  // namespace Kiran