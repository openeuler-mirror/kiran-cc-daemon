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
#include <glib/gi18n.h>

#include "lib/base/base.h"
#include "plugins/keybinding/shortcut-helper.h"

namespace Kiran
{
#define CUSTOM_SHORTCUT_KIND _("Custom")

struct CustomShortCut
{
    CustomShortCut() = default;
    CustomShortCut(const std::string &u, const std::string &n, const std::string a, const std::string &k) : uid(u),
                                                                                                            name(n),
                                                                                                            action(a),
                                                                                                            key_combination(k) {}
    CustomShortCut(const std::string &n, const std::string a, const std::string &k) : name(n),
                                                                                      action(a),
                                                                                      key_combination(k) {}

    // 快捷键的uid，对应keyfile文件中的group name
    std::string uid;
    // 快捷键名称
    std::string name;
    // 触发快捷键后执行的命令
    std::string action;
    // 快捷键
    std::string key_combination;
};

class CustomShortCuts
{
public:
    CustomShortCuts();
    virtual ~CustomShortCuts();

    // 初始化
    void init();

    // 添加自定义快捷键
    bool add(std::shared_ptr<CustomShortCut> shortcut);
    // 修改自定义快捷键，如果uid不存在则返回错误
    bool modify(std::shared_ptr<CustomShortCut> shortcut);
    // 删除自定义快捷键
    bool remove(const std::string &uid);
    // 获取自定义快捷键
    std::shared_ptr<CustomShortCut> get(const std::string &uid);
    // 通过keycomb搜索快捷键，如果存在多个快捷键有相同的keycomb，则返回第一个找到的快捷键
    std::shared_ptr<CustomShortCut> get_by_keycomb(const std::string &keycomb);
    // 获取所有自定义快捷键
    std::map<std::string, std::shared_ptr<CustomShortCut>> get();

    // 自定义快捷键变化信号。
    // 如果第一个CustomShortCut为空，则为新增快捷键；
    // 如果第二个CustomShortCut为空，则为删除快捷键；
    // 如果两个CustomShortCut都不为空，则为修改快捷键；
    // sigc::signal<void, std::shared_ptr<CustomShortCut>, std::shared_ptr<CustomShortCut>> signal_custom_shortcut_changed() const { return this->custom_shortcut_changed_; }

private:
    // 初始化需要使用的修饰符
    void init_modifiers();
    // 生成自定义快捷键唯一ID
    std::string gen_uid();
    // 校验自定义快捷键是否合法
    bool check_valid(std::shared_ptr<CustomShortCut> shortcut);
    // 设置自定义快捷键，如果uid不存在则创建，不做合法性校验
    void change_and_save(std::shared_ptr<CustomShortCut> shortcut, bool is_remove = false);
    // 保存自定义按键到文件
    bool save_to_file();
    // 抓取组合按键或者取消抓取组合按键
    bool grab_keycomb_change(const std::string &key_comb, bool is_grab);

    static GdkFilterReturn window_event(GdkXEvent *gdk_event, GdkEvent *event, gpointer data);

private:
    uint32_t ignored_mods_;
    uint32_t used_mods_;

    Glib::Rand rand_;

    std::string conf_file_path_;

    Glib::KeyFile keyfile_;

    sigc::connection save_id_;

    Glib::RefPtr<Gdk::Window> root_window_;

    // sigc::signal<void, std::shared_ptr<CustomShortCut>, std::shared_ptr<CustomShortCut>> custom_shortcut_changed_;
};
}  // namespace Kiran