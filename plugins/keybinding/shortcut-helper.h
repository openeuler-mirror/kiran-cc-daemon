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
