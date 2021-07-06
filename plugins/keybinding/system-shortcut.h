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

#include "lib/base/base.h"
#include "plugins/keybinding/keylist-entries-parser.h"
#include "plugins/keybinding/shortcut-helper.h"

namespace Kiran
{
struct SystemShortCut
{
    std::string uid;
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
    bool modify(const std::string &uid, const std::string &key_combination, CCErrorCode &error_code);
    // 获取系统快捷键
    std::shared_ptr<SystemShortCut> get(const std::string &uid);

    const std::map<std::string, std::shared_ptr<SystemShortCut>> &get() const { return this->shortcuts_; }

    // 新增系统快捷键
    sigc::signal<void, std::shared_ptr<SystemShortCut>> signal_shortcut_added() const { return this->shortcut_added_; }
    // 删除系统快捷键
    sigc::signal<void, std::shared_ptr<SystemShortCut>> signal_shortcut_deleted() const { return this->shortcut_deleted_; }
    // 系统快捷键修改
    sigc::signal<void, std::shared_ptr<SystemShortCut>> signal_shortcut_changed() const { return this->shortcut_changed_; }

private:
    void init();

    void load_system_shortcuts(std::map<std::string, std::shared_ptr<SystemShortCut>> &shortcuts);

    void wm_window_changed();

    void settings_changed(const Glib::ustring &key, const Glib::RefPtr<Gio::Settings> settings);

    bool should_show_key(const KeyListEntry &entry);

private:
    static SystemShortCutManager *instance_;

    sigc::signal<void, std::shared_ptr<SystemShortCut>> shortcut_added_;
    sigc::signal<void, std::shared_ptr<SystemShortCut>> shortcut_deleted_;
    sigc::signal<void, std::shared_ptr<SystemShortCut>> shortcut_changed_;

    std::map<std::string, std::shared_ptr<SystemShortCut>> shortcuts_;
};

}  // namespace Kiran