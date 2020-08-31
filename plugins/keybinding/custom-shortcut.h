/*
 * @Author       : tangjie02
 * @Date         : 2020-08-27 11:05:53
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-08-27 17:57:21
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/plugins/keybinding/custom-shortcut.h
 */
#pragma once

#include <gdkmm.h>
#include <glib/gi18n.h>

#include "lib/cc-dbus-error.h"
#include "plugins/keybinding/shortcut-helper.h"

namespace Kiran
{
#define CUSTOM_SHORTCUT_KIND _("Custom")

struct CustomShortCut
{
    CustomShortCut() = default;
    CustomShortCut(const std::string &n, const std::string k, const std::string &a) : name(n),
                                                                                      key_combination(k),
                                                                                      action(a) {}
    std::string name;
    std::string key_combination;
    std::string action;
};

class CustomShortCutManager
{
public:
    CustomShortCutManager();
    virtual ~CustomShortCutManager();

    static CustomShortCutManager *get_instance() { return instance_; };

    static void global_init();

    static void global_deinit() { delete instance_; };

    // 添加自定义快捷键
    std::pair<std::string, CCError> add(std::shared_ptr<CustomShortCut> custom_shortcut, std::string &err);
    // 修改自定义快捷键，如果uid不存在则返回错误
    CCError modify(const std::string &uid, std::shared_ptr<CustomShortCut> custom_shortcut, std::string &err);
    // 删除自定义快捷键
    CCError remove(const std::string &uid, std::string &err);
    // 获取自定义快捷键
    std::shared_ptr<CustomShortCut> get(const std::string &uid);
    // 获取所有自定义快捷键
    std::map<std::string, std::shared_ptr<CustomShortCut>> get();

    // 自定义快捷键变化信号。
    // 如果第一个CustomShortCut为空，则为新增快捷键；
    // 如果第二个CustomShortCut为空，则为删除快捷键；
    // 如果两个CustomShortCut都不为空，则为修改快捷键；
    // sigc::signal<void, std::shared_ptr<CustomShortCut>, std::shared_ptr<CustomShortCut>> signal_custom_shortcut_changed() const { return this->custom_shortcut_changed_; }

private:
    void init();
    // 生成自定义快捷键唯一ID
    std::string gen_uid();
    // 校验自定义快捷键是否合法
    bool check_valid(std::shared_ptr<CustomShortCut> custom_shortcut, std::string &err);
    // 设置自定义快捷键，如果uid不存在则创建，不做合法性校验
    void change_and_save(const std::string &uid, std::shared_ptr<CustomShortCut> custom_shortcut);
    bool save_to_file();

    static GdkFilterReturn window_event(GdkXEvent *xevent, GdkEvent *event, gpointer data);

private:
    static CustomShortCutManager *instance_;

    Glib::Rand rand_;

    std::string conf_file_path_;

    Glib::KeyFile keyfile_;

    sigc::connection save_id_;

    Glib::RefPtr<Gdk::Window> root_window_;

    // sigc::signal<void, std::shared_ptr<CustomShortCut>, std::shared_ptr<CustomShortCut>> custom_shortcut_changed_;
};
}  // namespace Kiran