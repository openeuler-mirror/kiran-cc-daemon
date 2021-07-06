/**
 * @Copyright (C) 2020 ~ 2021 KylinSec Co., Ltd. 
 *
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; If not, see <http: //www.gnu.org/licenses/>. 
 */

#pragma once

#include <touchpad_dbus_stub.h>

#include "plugins/inputdevices/common/device-helper.h"
#include "touchpad-i.h"

namespace Kiran
{
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

    virtual bool has_touchpad_setHandler(bool value) { return true; };
    virtual bool left_handed_setHandler(bool value);
    virtual bool disable_while_typing_setHandler(bool value);
    virtual bool tap_to_click_setHandler(bool value);
    virtual bool click_method_setHandler(gint32 value);
    virtual bool scroll_method_setHandler(gint32 value);
    virtual bool natural_scroll_setHandler(bool value);
    virtual bool touchpad_enabled_setHandler(bool value);
    virtual bool motion_acceleration_setHandler(double value);

    // 是否存在触摸板设备
    virtual bool has_touchpad_get() { return this->has_touchpad_; };
    // 左手模式，如果为真，那么触摸板左键和右键效果切换，这里只会切换BUTTON_AREAS模式下点击
    // 触摸板下方左右区域触发的左键和右键效果，不会切换1/2指轻击触摸板触发的效果
    virtual bool left_handed_get() { return this->left_handed_; };
    // 在用键盘打字时触摸板无效
    virtual bool disable_while_typing_get() { return this->disable_while_typing_; };
    // 敲击触摸板和鼠标按键之间的映射行为。默认情况下1/2/3指敲击分别对应鼠标左键/右键/中键
    virtual bool tap_to_click_get() { return this->tap_to_click_; };
    // 设置点击触摸板的方式，当设置为BUTTON_AREAS(0)时，触摸板下方会划分左中右三块区域，
    // 这三块区域分别对应鼠标左/中/右键，当然你也可以通过1/2/3指轻击上方区域触发左/右/中键；
    // 当设置为CLICK_FINGER(1)时，只能通过1/2/3指轻击触摸板(不划分上下区域)来触发左键/右键/中键
    virtual gint32 click_method_get() { return this->click_method_; };
    // 滚动窗口的方式，分为twofinger, edge和button三种方式。
    // twofinger表示用两指滑动触摸板来达到滚动效果；edge表示滑动触摸板右边边缘来达到滚动效果；button表示操作键盘中间的红色按钮(部分机型存在)来达到滚动效果。
    virtual gint32 scroll_method_get() { return this->scroll_method_; };
    // 自然滚动，如果设置为false，触摸板滑动方向与页面滚动方向相同，否则相反。具体的滚动方式可以通过设置scroll method来实现。
    virtual bool natural_scroll_get() { return this->natural_scroll_; };
    // 开启或禁用所有触摸板
    virtual bool touchpad_enabled_get() { return this->touchpad_enabled_; };
    // 移动加速，范围为[-1,1]
    virtual double motion_acceleration_get() { return this->motion_acceleration_; };

private:
    void init();

    void load_from_settings();
    void settings_changed(const Glib::ustring &key);

    void set_all_props_to_devices();
    void set_left_handed_to_devices();
    void set_disable_while_typing_to_devices();
    void set_tap_to_click_to_devices();
    void set_click_method_to_devices();
    void set_scroll_method_to_devices();
    void set_natural_scroll_to_devices();
    void set_touchpad_enabled_to_devices();
    void set_motion_acceleration_to_devices();

    void on_bus_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name);
    void on_name_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name);
    void on_name_lost(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name);

private:
    static TouchPadManager *instance_;

    uint32_t dbus_connect_id_;
    uint32_t object_register_id_;

    Glib::RefPtr<Gio::Settings> touchpad_settings_;

    bool has_touchpad_;
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
