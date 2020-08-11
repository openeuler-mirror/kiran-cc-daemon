/*
 * @Author       : tangjie02
 * @Date         : 2020-08-11 16:21:04
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-08-11 16:58:25
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/plugins/xsettings/xsettings-manager.h
 */

#pragma once

#include <xsettings_dbus_stub.h>

namespace Kiran
{
class XSettingsManager : public SessionDaemon::XSettingsStub
{
public:
    XSettingsManager();
    virtual ~XSettingsManager();

    static XSettingsManager *get_instance() { return instance_; };

    static void global_init();

    static void global_deinit() { delete instance_; };

protected:
    virtual bool double_click_time_setHandler(gint32 value);
    virtual gint32 double_click_time_get() { return this->double_click_time_; };

private:
    void init();

    void load_from_settings();
    void settings_changed(const Glib::ustring &key);

    void on_bus_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name);
    void on_name_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name);
    void on_name_lost(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name);

private:
    static XSettingsManager *instance_;

    uint32_t dbus_connect_id_;
    uint32_t object_register_id_;

    Glib::RefPtr<Gio::Settings> mouse_settings_;

    int32_t double_click_time_;
};
}  // namespace Kiran