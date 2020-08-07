/*
 * @Author       : tangjie02
 * @Date         : 2020-06-19 10:08:59
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-08-07 11:18:32
 * @Description  : 
 * @FilePath     : /kiran-system-daemon/plugins/inputdevices/touchpad/touchpad-manager.h
 */

#pragma once

#include <touchpad_dbus_stub.h>

#include "plugins/inputdevices/common/device-helper.h"

namespace Kiran
{
enum class ClickMethod : int32_t
{

    BUTTON_AREAS,
    CLICK_FINGER,
};

enum class ScrollMethod
{
    TWO_FINGER,
    EDGE,
    BUTTON,
};

class TouchPadManager : public SessionDaemon::TouchPadStub
{
public:
    TouchPadManager();
    virtual ~TouchPadManager();

    static TouchPadManager *get_instance() { return instance_; };

    static void global_init();

    static void global_deinit() { delete instance_; };

protected:
    virtual void Reset(MethodInvocation &invocation);

    // 设置左手模式，如果为真，那么触摸板单击和双击效果切换
    virtual bool left_handed_setHandler(bool value);
    // 设置在用键盘打字时触摸板无效
    virtual bool disable_while_typing_setHandler(bool value);
    // 开启敲击触摸板和鼠标按键之间的映射行为。默认情况下1/2/3指敲击分别对应鼠标左键/右键/中键
    virtual bool tap_to_click_setHandler(bool value);
    // 设置点击触摸板的方式，当设置为BUTTON_AREAS时，触摸板划分左中右三块区域，按下这三块区域分别对应左键/右键/中键；当设置为CLICK_FINGER时，可以通过1/2/3指轻击触摸板来触发左键/右键/中键
    virtual bool click_method_setHandler(gint32 value);
    // 滚动窗口的方式，分为twofinger, edge和button三种方式。
    // twofinger表示用两指滑动触摸板来达到滚动效果；edge表示滑动触摸板右边边缘来达到滚动效果；button表示操作键盘中间的红色按钮(部分机型存在)来达到滚动效果。
    virtual bool scroll_method_setHandler(gint32 value);
    // 设置自然滚动，开启自然滚动后可以滚动屏幕中的窗口内容，具体的滚动方式可以通过设置scroll method来实现。
    virtual bool natural_scroll_setHandler(bool value);
    // 开启或禁用所有触摸板
    virtual bool touchpad_enabled_setHandler(bool value);
    virtual bool motion_acceleration_setHandler(double value);

    virtual bool left_handed_get() { return this->left_handed_; };
    virtual bool disable_while_typing_get() { return this->disable_while_typing_; };
    virtual bool tap_to_click_get() { return this->tap_to_click_; };
    virtual gint32 click_method_get() { return this->click_method_; };
    virtual gint32 scroll_method_get();
    virtual bool natural_scroll_get();
    virtual bool touchpad_enabled_get();
    virtual double motion_acceleration_get();
    virtual gint32 motion_threshold_get();

private:
    void init();

    void load_from_settings();
    void settings_changed(const Glib::ustring &key);

    void set_left_handed_to_devices();
    void set_disable_while_typing_to_devices();
    void set_tap_to_click_to_devices();
    void set_click_method_to_devices();
    void set_scroll_method_to_devices();
    void set_natural_scroll_to_devices();
    void set_touchpad_enabled_to_devices();
    void set_motion_acceleration_to_devices();

    void set_left_handed_to_device(std::shared_ptr<DeviceHelper> device_helper);
    void set_disable_while_typing_to_device(std::shared_ptr<DeviceHelper> device_helper);
    void set_tap_to_click_to_device(std::shared_ptr<DeviceHelper> device_helper);
    void set_click_method_to_device(std::shared_ptr<DeviceHelper> device_helper);
    void set_scroll_method_to_device(std::shared_ptr<DeviceHelper> device_helper);
    void set_natural_scroll_to_device(std::shared_ptr<DeviceHelper> device_helper);
    void set_touchpad_enabled_to_device(std::shared_ptr<DeviceHelper> device_helper);
    void set_motion_acceleration_to_device(std::shared_ptr<DeviceHelper> device_helper);

    void on_bus_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name);
    void on_name_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name);
    void on_name_lost(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name);

private:
    static TouchPadManager *instance_;

    uint32_t dbus_connect_id_;
    uint32_t object_register_id_;

    Glib::RefPtr<Gio::Settings> touchpad_settings_;

    bool left_handed_;
    bool disable_while_typing_;
    bool tap_to_click_;
    int32_t click_method_;
    int32_t scroll_method_;
    bool natural_scroll_;
    bool touchpad_enabled_;
    double motion_acceleration_;
};
}  // namespace Kiran