/*
 * @Author       : tangjie02
 * @Date         : 2020-08-26 11:27:37
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-08-27 18:17:37
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/plugins/keybinding/shortcut-helper.cpp
 */

#include "plugins/keybinding/shortcut-helper.h"

#include <gdk/gdkkeysyms.h>
#include <gdk/gdkx.h>

#include "lib/helper.h"
#include "lib/str-util.h"

namespace Kiran
{
KeyState ShortCutHelper::get_key_state(const std::string &key_comb)
{
    RETURN_VAL_IF_TRUE(key_comb.length() == 0, NULL_KEYSTATE);
    RETURN_VAL_IF_TRUE(StrUtil::tolower(key_comb) == "disabled", NULL_KEYSTATE);

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
            auto token = StrUtil::tolower(key_comb.substr(cur_pos, gt_pos - cur_pos + 1));
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
                break;
            }
        }
        else
        {
            auto keyval = gdk_keyval_from_name(key_comb.substr(cur_pos).c_str());
            RETURN_VAL_IF_TRUE(keyval == GDK_KEY_VoidSymbol, INVALID_KEYSTATE);
            key_state.key_symbol = keyval;
            break;
        }
    }
    return key_state;
}
}  // namespace Kiran