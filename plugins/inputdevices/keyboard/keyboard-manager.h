/*
 * @Author       : tangjie02
 * @Date         : 2020-08-12 16:25:44
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-08-20 11:24:16
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/plugins/inputdevices/keyboard/keyboard-manager.h
 */
#pragma once

#include <keyboard_dbus_stub.h>

#include "plugins/inputdevices/common/device-helper.h"

namespace Kiran
{
class KeyboardManager : public SessionDaemon::KeyboardStub
{
public:
    KeyboardManager();
    virtual ~KeyboardManager();

    static KeyboardManager *get_instance() { return instance_; };

    static void global_init();

    static void global_deinit() { delete instance_; };

protected:
    // 添加键盘布局。键盘布局最多只能设置4个，如果超过4个则返回添加；
    // 如果布局不在GetValidLayouts返回的列表中，或者布局已经存在用户布局列表中，则返回添加失败；
    // 如果设置布局命令执行错误，也返回添加失败；
    // 否则返回添加成功。
    virtual void AddLayout(const Glib::ustring &layout, MethodInvocation &invocation);

    // 从用户布局列表中删除键盘布局，如果用户布局列表中不存在该布局，则返回删除失败；
    // 如果设置布局命令执行错误，也返回删除失败；
    // 否则返回删除成功
    virtual void DelLayout(const Glib::ustring &layout, MethodInvocation &invocation);

    // 获取所有合法的键盘布局列表，这个列表是从/usr/share/X11/xkb/rules/base.xml中读取。
    virtual void GetValidLayouts(MethodInvocation &invocation);

    // 添加布局选项
    virtual void AddLayoutOption(const Glib::ustring &option, MethodInvocation &invocation);
    // 删除布局选项
    virtual void DelLayoutOption(const Glib::ustring &option, MethodInvocation &invocation);
    // 清理布局选项
    virtual void ClearLayoutOption(MethodInvocation &invocation);

    virtual bool repeat_enabled_setHandler(bool value);
    virtual bool repeat_delay_setHandler(gint32 value);
    virtual bool repeat_interval_setHandler(gint32 value);
    virtual bool layouts_setHandler(const std::vector<Glib::ustring> &value);
    virtual bool options_setHandler(const std::vector<Glib::ustring> &value);

    virtual bool repeat_enabled_get() { return this->repeat_enabled_; };
    virtual gint32 repeat_delay_get() { return this->repeat_delay_; };
    virtual gint32 repeat_interval_get() { return this->repeat_interval_; };
    virtual std::vector<Glib::ustring> layouts_get() { return this->layouts_; };
    virtual std::vector<Glib::ustring> options_get() { return this->options_; };

private:
    void init();

    void load_from_settings();
    void settings_changed(const Glib::ustring &key);

    void load_xkb_rules();

    void set_all_props();
    void set_auto_repeat();
    bool set_layouts(const std::vector<Glib::ustring> &layouts);
    bool set_options(const std::vector<Glib::ustring> &options);

    void on_bus_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name);
    void on_name_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name);
    void on_name_lost(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name);

private:
    static KeyboardManager *instance_;

    uint32_t dbus_connect_id_;
    uint32_t object_register_id_;

    Glib::RefPtr<Gio::Settings> keyboard_settings_;
    std::map<Glib::ustring, Glib::ustring> valid_layouts_;

    bool repeat_enabled_;
    int32_t repeat_delay_;
    int32_t repeat_interval_;
    std::vector<Glib::ustring> layouts_;
    std::vector<Glib::ustring> options_;
};

}  // namespace Kiran