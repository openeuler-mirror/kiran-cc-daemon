/*
 * @Author       : tangjie02
 * @Date         : 2020-08-24 16:19:21
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-09-02 20:39:50
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/plugins/keybinding/keybinding-manager.h
 */

#pragma once

#include <gdkmm.h>
#include <keybinding_dbus_stub.h>

#include "plugins/keybinding/custom-shortcut.h"
#include "plugins/keybinding/system-shortcut.h"

namespace Kiran
{
class KeybindingManager : public SessionDaemon::KeybindingStub
{
public:
    KeybindingManager(SystemShortCutManager *system);
    virtual ~KeybindingManager();

    static KeybindingManager *get_instance() { return instance_; };

    static void global_init(SystemShortCutManager *system);

    static void global_deinit() { delete instance_; };

protected:
    // 添加自定义快捷键
    virtual void AddCustomShortcut(const Glib::ustring &name,
                                   const Glib::ustring &action,
                                   const Glib::ustring &key_combination,
                                   MethodInvocation &invocation);
    // 修改自定义快捷键
    virtual void ModifyCustomShortcut(const Glib::ustring &uid,
                                      const Glib::ustring &name,
                                      const Glib::ustring &action,
                                      const Glib::ustring &key_combination,
                                      MethodInvocation &invocation);
    // 删除自定义快捷键
    virtual void DeleteCustomShortcut(const Glib::ustring &uid, MethodInvocation &invocation);
    // 获取指定自定义快捷键
    virtual void GetCustomShortcut(const Glib::ustring &uid, MethodInvocation &invocation);
    // 获取所有自定义快捷键
    virtual void ListCustomShortcuts(MethodInvocation &invocation);
    // 修改系统快捷键
    virtual void ModifySystemShortcut(const Glib::ustring &uid,
                                      const Glib::ustring &key_combination,
                                      MethodInvocation &invocation);
    // 获取指定系统快捷键
    virtual void GetSystemShortcut(const Glib::ustring &uid, MethodInvocation &invocation);
    // 获取所有系统快捷键
    virtual void ListSystemShortcuts(MethodInvocation &invocation);
    // 获取所有快捷键，包括自定义和系统快捷键
    virtual void ListShortcuts(MethodInvocation &invocation);

private:
    using ShortCutTuple = std::tuple<std::string, std::string, std::string>;

private:
    void init();

    void system_shortcut_added(std::shared_ptr<SystemShortCut> system_shortcut);
    void system_shortcut_deleted(std::shared_ptr<SystemShortCut> system_shortcut);
    void system_shortcut_changed(std::shared_ptr<SystemShortCut> system_shortcut);

    void on_bus_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name);
    void on_name_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name);
    void on_name_lost(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name);

private:
    static KeybindingManager *instance_;

    SystemShortCutManager *system_shorcut_manager_;

    uint32_t dbus_connect_id_;
    uint32_t object_register_id_;
};
}  // namespace Kiran