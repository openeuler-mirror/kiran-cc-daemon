/**
 * Copyright (c) 2020 ~ 2021 KylinSec Co., Ltd. 
 * kiran-cc-daemon is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2. 
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2 
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, 
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, 
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.  
 * See the Mulan PSL v2 for more details.  
 * 
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
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

    virtual bool left_handed_setHandler(bool value);
    virtual bool motion_acceleration_setHandler(double value);
    virtual bool middle_emulation_enabled_setHandler(bool value);
    virtual bool natural_scroll_setHandler(bool value);

    // 左手模式，会对鼠标左键和右键的功能进行互换
    virtual bool left_handed_get() { return this->left_handed_; };
    // 移动加速，范围为[-1,1]
    virtual double motion_acceleration_get() { return this->motion_acceleration_; };
    // 开启鼠标滚动键仿真效果，通过同时点击鼠标左键和右键来触发滚轮点击事件
    virtual bool middle_emulation_enabled_get() { return this->middle_emulation_enabled_; };
    // 自然滚动，如果设置为false（默认值），那么鼠标滚轮向下时页面滚动也向下；如果设置为true，那么鼠标滚轮向下时页面滚动则向上。
    virtual bool natural_scroll_get() { return this->natural_scroll_; };

private:
    void init();

    void load_from_settings();
    void settings_changed(const Glib::ustring &key);

    void set_all_props_to_devices();
    void set_left_handed_to_devices();
    void set_motion_acceleration_to_devices();
    void set_middle_emulation_enabled_to_devices();
    void set_natural_scroll_to_devices();

    void foreach_device(std::function<void(std::shared_ptr<DeviceHelper>)> callback);

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
    bool middle_emulation_enabled_;
    bool natural_scroll_;
};
}  // namespace Kiran