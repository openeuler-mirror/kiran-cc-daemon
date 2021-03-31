/*
 * @Author       : tangjie02
 * @Date         : 2020-08-27 11:05:53
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-09-02 15:25:16
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/plugins/keybinding/custom-shortcut.h
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
    CustomShortCut(const std::string &n, const std::string a, const std::string &k) : name(n),
                                                                                      action(a),
                                                                                      key_combination(k)
    {
    }
    std::string name;
    std::string action;
    std::string key_combination;
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
    std::string add(std::shared_ptr<CustomShortCut> shortcut, CCErrorCode &error_code);
    // 修改自定义快捷键，如果uid不存在则返回错误
    bool modify(const std::string &uid, std::shared_ptr<CustomShortCut> shortcut, CCErrorCode &error_code);
    // 删除自定义快捷键
    bool remove(const std::string &uid, CCErrorCode &error_code);
    // 获取自定义快捷键
    std::shared_ptr<CustomShortCut> get(const std::string &uid);
    // 获取所有自定义快捷键
    std::map<std::string, std::shared_ptr<CustomShortCut>> get();
    // 通过keycomb搜索快捷键
    std::string lookup_shortcut(const std::string &keycomb);

    // 自定义快捷键变化信号。
    // 如果第一个CustomShortCut为空，则为新增快捷键；
    // 如果第二个CustomShortCut为空，则为删除快捷键；
    // 如果两个CustomShortCut都不为空，则为修改快捷键；
    // sigc::signal<void, std::shared_ptr<CustomShortCut>, std::shared_ptr<CustomShortCut>> signal_custom_shortcut_changed() const { return this->custom_shortcut_changed_; }

private:
    void init();
    // 初始化需要使用的修饰符
    void init_modifiers();
    // 获取NumLock修饰键，NumLock修饰键可能对应Mod1-Mod5中的其中一个。
    uint32_t get_numlock_modifier();
    // 生成自定义快捷键唯一ID
    std::string gen_uid();
    // 校验自定义快捷键是否合法
    bool check_valid(std::shared_ptr<CustomShortCut> shortcut, CCErrorCode &error_code);
    // 设置自定义快捷键，如果uid不存在则创建，不做合法性校验
    void change_and_save(const std::string &uid, std::shared_ptr<CustomShortCut> shortcut);
    // 保存自定义按键到文件
    bool save_to_file();
    // 抓取组合按键或者取消抓取组合按键
    bool grab_keycomb_change(const std::string &key_comb, bool grab, CCErrorCode &error_code);
    bool grab_keystate_change(const KeyState &keystate, bool grab, CCErrorCode &error_code);

    static GdkFilterReturn window_event(GdkXEvent *gdk_event, GdkEvent *event, gpointer data);

private:
    static CustomShortCutManager *instance_;

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