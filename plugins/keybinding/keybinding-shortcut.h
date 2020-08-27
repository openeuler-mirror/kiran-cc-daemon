/*
 * @Author       : tangjie02
 * @Date         : 2020-08-26 11:27:32
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-08-26 17:03:57
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/plugins/keybinding/keybinding-shortcut.h
 */

#pragma once

#include <giomm.h>

#include <memory>
#include <string>

namespace Kiran
{
#define INVALID_KEYSTATE KeyState()
#define SHORTCUT_KIND_CUSTOM "Custom"

struct KeyState
{
    KeyState() : key_symbol(0), mods(0) {}
    uint32_t key_symbol;
    uint32_t mods;
    bool operator==(const KeyState &key_state) const
    {
        return key_symbol == key_state.key_symbol && mods == key_state.mods;
    }
};

enum class ShortCutType : int32_t
{
    SYSTEM,
    CUSTOM
};

class KeybindingManager;

class ShortCut
{
public:
    ShortCut() = default;
    ShortCut(const std::string &kind, const std::string &name, const std::string &key_combination = std::string());
    virtual ~ShortCut(){};

    virtual bool is_valid() const;

protected:
    std::string kind_;
    std::string name_;
    std::string key_combination_;
    friend class KeybindingManager;
};

class CustomShortCut : public ShortCut
{
private:
    std::string action_;
    KeyState key_state_;
    friend class KeybindingManager;
};

class SystemShortCut : public ShortCut
{
public:
    SystemShortCut(const std::string &kind,
                   const std::string &name,
                   const std::string &settings_path,
                   const std::string &settings_key,
                   const std::string &package);

    virtual ~SystemShortCut(){};

    virtual bool is_valid() const override;

    std::string get_id() { return this->id_; }

    sigc::signal<void, std::string, std::string> &signal_keycomb_changed() { return this->keycomb_changed_; }

private:
    void init();

    void settings_changed(const Glib::ustring &key);

private:
    std::string settings_path_;
    std::string settings_key_;
    std::string package_;
    Glib::RefPtr<Gio::Settings> settings_;
    sigc::signal<void, std::string, std::string> keycomb_changed_;
    std::string id_;
    friend class KeybindingManager;
};

}  // namespace  Kiran
