/*
 * @Author       : tangjie02
 * @Date         : 2020-06-19 10:08:59
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-08-07 11:17:21
 * @Description  : 
 * @FilePath     : /kiran-system-daemon/plugins/inputdevices/mouse/mouse-manager.h
 */

#pragma once

#include <mouse_dbus_stub.h>

#include "plugins/inputdevices/common/device-helper.h"

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

    // 设置左手模式，会对鼠标左键和右键的效果进行切换
    virtual bool left_handed_setHandler(bool value);
    // 设置鼠标加速，范围为[1,10]
    virtual bool motion_acceleration_setHandler(double value);
    // 暂不支持
    virtual bool double_click_setHandler(gint32 value);
    // 开启鼠标中键仿真效果，通过同时点击左键和右键来触发中键点击效果
    virtual bool middle_emulation_enabled_setHandler(bool value);
    // 开启自然滚动后可以通过鼠标滚动键（中键）滚动窗口
    virtual bool natural_scroll_setHandler(bool value);

    virtual bool left_handed_get() { return this->left_handed_; };
    virtual double motion_acceleration_get() { return this->motion_acceleration_; };
    virtual gint32 double_click_get() { return this->double_click_; };
    virtual bool middle_emulation_enabled_get() { return this->middle_emulation_enabled_; };
    virtual bool natural_scroll_get();

private:
    void init();

    void load_from_settings();
    void settings_changed(const Glib::ustring &key);

    void set_left_handed_to_devices();
    void set_motion_acceleration_to_devices();
    void set_middle_emulation_enabled_to_devices();
    void set_natural_scroll_to_devices();

    void set_left_handed_to_device(std::shared_ptr<DeviceHelper> device_helper);
    void set_motion_acceleration_to_device(std::shared_ptr<DeviceHelper> device_helper);
    void set_middle_emulation_enabled_to_device(std::shared_ptr<DeviceHelper> device_helper);
    void set_natural_scroll_to_device(std::shared_ptr<DeviceHelper> device_helper);

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
    bool natural_scroll_;
};
}  // namespace Kiran