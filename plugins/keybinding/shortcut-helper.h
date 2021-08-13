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

#pragma once

#include <gdkmm.h>
//
#include <X11/Xlib.h>

#include <memory>
#include <string>
namespace Kiran
{
#define INVALID_KEYSTATE KeyState(0xFFFFFFFF, 0xFFFFFFFF)
#define NULL_KEYSTATE KeyState()
#define SHORTCUT_KIND_CUSTOM "Custom"
#define SHORTCUT_KEYCOMB_DISABLE "disabled"
#define KEYBINDING_CONF_DIR "kylinsec/kiran/session-daemon/keybinding"

struct KeyState
{
    KeyState(uint32_t k = 0, uint32_t m = 0) : key_symbol(k), mods(m) {}
    uint32_t key_symbol;
    uint32_t mods;
    std::vector<uint32_t> keycodes;
    bool operator==(const KeyState &key_state) const
    {
        return key_symbol == key_state.key_symbol && mods == key_state.mods;
    }

    bool operator!=(const KeyState &key_state) const
    {
        return !(this->operator==(key_state));
    }
};

enum class ShortCutType : int32_t
{
    SYSTEM,
    CUSTOM
};

class ShortCutHelper
{
public:
    using KeyCodeFilter = std::function<bool(int, int)>;

public:
    ShortCutHelper(){};
    virtual ~ShortCutHelper(){};

    static KeyState get_keystate(const std::string &key_comb);
    static KeyState get_keystate(XEvent *event);
    static std::vector<uint32_t> get_keycode(uint32_t key_symbol, KeyCodeFilter filter);
};

}  // namespace  Kiran
