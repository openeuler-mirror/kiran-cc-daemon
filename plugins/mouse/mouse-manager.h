/*
 * @Author       : tangjie02
 * @Date         : 2020-06-19 10:08:59
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-08-06 15:36:56
 * @Description  : 
 * @FilePath     : /kiran-system-daemon/plugins/mouse/mouse-manager.h
 */

#pragma once

#include <mouse_dbus_stub.h>

#include "lib/device-helper.h"

namespace Kiran
{
class MouseManager : public SessionDaemon::MouseStub
{
public:
    MouseManager();
    virtual ~MouseManager();

    static MouseManager *get_instance() { return instance_; };

    static void global_init();

    static void global_deinit() { delete instance_; };

protected:
    virtual void Reset(MethodInvocation &invocation);

    virtual bool left_handed_setHandler(bool value);
    virtual bool motion_acceleration_setHandler(double value);
    virtual bool double_click_setHandler(gint32 value);
    virtual bool middle_button_enabled_setHandler(bool value);
    virtual bool middle_emulation_enabled_setHandler(bool value);

    virtual bool left_handed_get() { return this->left_handed_; };
    virtual double motion_acceleration_get() { return this->motion_acceleration_; };
    virtual gint32 double_click_get() { return this->double_click_; };
    virtual bool middle_emulation_enabled_get() { return this->middle_emulation_enabled_; };

private:
    void init();

    void load_from_settings();
    void settings_changed(const Glib::ustring &key);

    void set_left_handed_to_devices();
    void set_motion_acceleration_to_devices();
    void set_middle_emulation_enabled_to_devices();

    void set_left_handed_to_device(std::shared_ptr<DeviceHelper> device_helper);
    void set_motion_acceleration_to_device(std::shared_ptr<DeviceHelper> device_helper);
    void set_middle_emulation_enabled_to_device(std::shared_ptr<DeviceHelper> device_helper);

    void on_bus_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name);
    void on_name_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name);
    void on_name_lost(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name);

private:
    static MouseManager *instance_;

    uint32_t dbus_connect_id_;
    uint32_t object_register_id_;

    Glib::RefPtr<Gio::Settings> mouse_settings_;

    bool left_handed_;
    double motion_acceleration_;
    int32_t double_click_;
    bool middle_emulation_enabled_;
};
}  // namespace Kiran