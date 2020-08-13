/*
 * @Author       : tangjie02
 * @Date         : 2020-08-12 16:25:44
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-08-13 09:55:34
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
    virtual void AddLayout(const Glib::ustring &layout, MethodInvocation &invocation);
    virtual void DelLayout(const Glib::ustring &layout, MethodInvocation &invocation);
    virtual void AddLayoutOption(const Glib::ustring &option, MethodInvocation &invocation);
    virtual void DelLayoutOption(const Glib::ustring &option, MethodInvocation &invocation);
    virtual void ClearLayoutOption(MethodInvocation &invocation);

    virtual bool repeat_enabled_setHandler(bool value);
    virtual bool repeat_delay_setHandler(gint32 value);
    virtual bool repeat_interval_setHandler(gint32 value);
    virtual bool model_setHandler(const Glib::ustring &value);
    virtual bool layouts_setHandler(const std::vector<Glib::ustring> &value);
    virtual bool options_setHandler(const std::vector<Glib::ustring> &value);

    virtual bool repeat_enabled_get() { return this->repeat_enabled_; };
    virtual gint32 repeat_delay_get() { return this->repeat_delay_; };
    virtual gint32 repeat_interval_get() { return this->repeat_interval_; };
    virtual Glib::ustring model_get() { return this->model_; };
    virtual std::vector<Glib::ustring> layouts_get() { return this->layouts_; };
    virtual std::vector<Glib::ustring> options_get() { return this->options_; };

private:
    void init();

    void load_from_settings();
    void settings_changed(const Glib::ustring &key);

    void set_all_props();
    void set_auto_repeat();

    void on_bus_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name);
    void on_name_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name);
    void on_name_lost(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name);

private:
    static KeyboardManager *instance_;

    uint32_t dbus_connect_id_;
    uint32_t object_register_id_;

    Glib::RefPtr<Gio::Settings> keyboard_settings_;

    bool repeat_enabled_;
    int32_t repeat_delay_;
    int32_t repeat_interval_;
    Glib::ustring model_;
    std::vector<Glib::ustring> layouts_;
    std::vector<Glib::ustring> options_;
};

}  // namespace Kiran