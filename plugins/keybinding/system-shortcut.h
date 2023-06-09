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

#include "lib/base/base.h"
#include "plugins/keybinding/keylist-entries-parser.h"
#include "plugins/keybinding/shortcut-helper.h"

namespace Kiran
{
struct SystemShortCut
{
    std::string uid;
    // 分类名
    std::string kind;
    // 快捷键名称
    std::string name;
    // 快捷键
    std::string key_combination;
    // 快捷键默认值
    std::string default_key_combination;
    // 读写快捷键的gsettings
    Glib::RefPtr<Gio::Settings> settings;
    // 快捷键在gsettings中的key
    std::string settings_key;
    // 感知gsettings变化的信号连接
    sigc::connection connection;
};

class SystemShortCuts: public sigc::trackable
{
public:
    SystemShortCuts();
    virtual ~SystemShortCuts(){};

    // 初始化
    void init();

    // 修改系统快捷键
    bool modify(const std::string &uid, const std::string &key_combination);
    // 获取系统快捷键
    std::shared_ptr<SystemShortCut> get(const std::string &uid);
    // 通过keycomb搜索快捷键，如果存在多个快捷键有相同的keycomb，则返回第一个找到的快捷键
    std::shared_ptr<SystemShortCut> get_by_keycomb(const std::string &keycomb);
    const std::map<std::string, std::shared_ptr<SystemShortCut>> &get() const { return this->shortcuts_; }
    // 重置系统快捷键
    void reset();

    // 新增系统快捷键
    sigc::signal<void, std::shared_ptr<SystemShortCut>> signal_shortcut_added() const { return this->shortcut_added_; }
    // 删除系统快捷键
    sigc::signal<void, std::shared_ptr<SystemShortCut>> signal_shortcut_deleted() const { return this->shortcut_deleted_; }
    // 系统快捷键修改
    sigc::signal<void, std::shared_ptr<SystemShortCut>> signal_shortcut_changed() const { return this->shortcut_changed_; }

private:
    // 根据配置文件加载系统快捷键
    void load_system_shortcuts(std::map<std::string, std::shared_ptr<SystemShortCut>> &shortcuts);

    void wm_window_changed();

    void settings_changed(const Glib::ustring &key, std::string shortcut_uid);

    bool should_show_key(const KeyListEntry &entry);

private:
    sigc::signal<void, std::shared_ptr<SystemShortCut>> shortcut_added_;
    sigc::signal<void, std::shared_ptr<SystemShortCut>> shortcut_deleted_;
    sigc::signal<void, std::shared_ptr<SystemShortCut>> shortcut_changed_;

    std::map<std::string, std::shared_ptr<SystemShortCut>> shortcuts_;
};

}  // namespace Kiran