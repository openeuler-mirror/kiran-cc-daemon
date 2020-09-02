/*
 * @Author       : tangjie02
 * @Date         : 2020-08-27 11:06:08
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-09-02 11:41:42
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/plugins/keybinding/system-shortcut.h
 */

#pragma once

#include "lib/cc-dbus-error.h"
#include "plugins/keybinding/keylist-entries-parser.h"
#include "plugins/keybinding/shortcut-helper.h"

namespace Kiran
{
struct SystemShortCut
{
    std::string kind;
    std::string name;
    std::string key_combination;
    Glib::RefPtr<Gio::Settings> settings;
    std::string settings_key;
};

class SystemShortCutManager
{
public:
    SystemShortCutManager();
    virtual ~SystemShortCutManager(){};

    static SystemShortCutManager *get_instance() { return instance_; };

    static void global_init();

    static void global_deinit() { delete instance_; };

    // 修改系统快捷键
    CCError modify(const std::string &uid,
                   const std::string &key_combination,
                   std::string &err);
    // 获取系统快捷键
    std::shared_ptr<SystemShortCut> get(const std::string &uid);

    const std::map<std::string, std::shared_ptr<SystemShortCut>> &get() const { return this->system_shortcuts_; }

    sigc::signal<void, const std::string &> signal_system_shortcut_changed() const { return this->system_shortcut_changed_; }

private:
    void init();

    void settings_changed(const Glib::ustring &key, const Glib::RefPtr<Gio::Settings> settings);

    bool should_show_key(const KeyListEntry &entry);

private:
    static SystemShortCutManager *instance_;

    sigc::signal<void, const std::string &> system_shortcut_changed_;

    std::map<std::string, std::shared_ptr<SystemShortCut>> system_shortcuts_;
};

}  // namespace Kiran