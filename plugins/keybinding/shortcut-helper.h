/*
 * @Author       : tangjie02
 * @Date         : 2020-08-26 11:27:32
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-09-02 09:51:40
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/plugins/keybinding/shortcut-helper.h
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
