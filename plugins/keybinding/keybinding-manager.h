/*
 * @Author       : tangjie02
 * @Date         : 2020-08-24 16:19:21
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-08-26 14:33:12
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/plugins/keybinding/keybinding-manager.h
 */

#pragma once

#include <gdkmm.h>
#include <keybinding_dbus_stub.h>

#include "plugins/keybinding/keybinding-shortcut.h"

namespace Kiran
{
class KeybindingManager : public SessionDaemon::KeybindingStub
{
public:
    KeybindingManager();
    virtual ~KeybindingManager();

    static KeybindingManager *get_instance() { return instance_; };

    static void global_init();

    static void global_deinit() { delete instance_; };

protected:
    // 添加自定义快捷键
    virtual void AddCustomShortcut(const Glib::ustring &name,
                                   const Glib::ustring &action,
                                   const Glib::ustring &key_combination,
                                   MethodInvocation &invocation);

    // 删除自定义快捷键
    virtual void DeleteCustomShortcut(const Glib::ustring &name, MethodInvocation &invocation);
    // 获取自定义快捷键
    virtual void ListCustomShortcuts(MethodInvocation &invocation);

private:
    using ShortCutTuple = std::tuple<std::string, std::string, std::string>;

private:
    void init();

    void load_custom_shortcut(std::map<std::string, std::shared_ptr<ShortCut>> &shortcuts);
    void custom_settings_changed(const Glib::ustring &key);

    void load_system_shortcut(std::map<std::string, std::shared_ptr<ShortCut>> &shortcuts);

    static GdkFilterReturn window_event(GdkXEvent *xevent, GdkEvent *event, gpointer data);
    KeyState get_key_state(GdkEvent *event);

    KeyState get_key_state(const std::string &keycomb);

    void on_bus_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name);
    void on_name_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name);
    void on_name_lost(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name);

private:
    static KeybindingManager *instance_;

    uint32_t dbus_connect_id_;
    uint32_t object_register_id_;

    Glib::RefPtr<Gdk::Window> root_window_;

    Glib::RefPtr<Gio::Settings> keybinding_settings_;

    std::map<std::string, std::shared_ptr<ShortCut>> shortcuts_;
};
}  // namespace Kiran