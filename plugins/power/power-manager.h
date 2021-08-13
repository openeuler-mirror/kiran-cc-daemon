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

#include <power_dbus_stub.h>

#include "lib/base/base.h"
#include "plugins/power/backlight/power-backlight.h"
#include "plugins/power/wrapper/power-wrapper-manager.h"

namespace Kiran
{
class PowerManager : public SessionDaemon::PowerStub
{
public:
    PowerManager(PowerWrapperManager *wrapper_manager, PowerBacklight *backlight);
    virtual ~PowerManager();

    static PowerManager *get_instance() { return instance_; };

    static void global_init(PowerWrapperManager *wrapper_manager, PowerBacklight *backlight);

    static void global_deinit() { delete instance_; };

protected:
    /**
     * @brief 设置计算机空闲时，在不同供电情况下(supply指定)，不同设备(device指定)的空闲超时(idle_timeout指定)处理动作(action指定)
     * @param {device} 可以指定为计算机或者背光设备，参考PowerDeviceType的定义
     * @param {supply} 系统供电方式，参考PowerSupplyMode的定义
     * @param {idle_timeout} 空闲超时时间，空闲时间超过该时间则触发动作
     * @param {action} 触发的动作，参考PowerAction的定义
     * @return {} 参数错误或者写入gsetetings失败则返回错误，否则返回成功
     */
    virtual void SetIdleAction(gint32 device,
                               gint32 supply,
                               gint32 idle_timeout,
                               gint32 action,
                               MethodInvocation &invocation);

    // 获取计算机空闲时的处理动作
    virtual void GetIdleAction(gint32 device,
                               gint32 supply,
                               MethodInvocation &invocation);

    /**
     * @brief 设置某个事件触发时的动作
     * @param {event} 事件类型，参考PowerEvent的定义
     * @param {action} 触发的动作，参考PowerAction的定义
     * @return 参数错误或者写入gsetetings失败则返回错误，否则返回成功
     */
    virtual void SetEventAction(gint32 event,
                                gint32 action,
                                MethodInvocation &invocation);

    // 获取某个事件触发时的动作
    virtual void GetEventAction(gint32 event,
                                MethodInvocation &invocation);

    /**
     * @brief 设置指定设备的亮度百分比。
     * @param {device} 可以设置为显示器或者键盘
     * @param {brightness_percentage} 亮度百分比，范围为[0, 100]
     * @return 如果参数错误或者不支持设置亮度则返回错误
     */
    virtual void SetBrightness(gint32 device,
                               gint32 brightness_percentage,
                               MethodInvocation &invocation);

    // 获取设备的亮度百分比，如果设备不支持则返回-1
    virtual void GetBrightness(gint32 device,
                               MethodInvocation &invocation);

    // 设置空闲时显示器变暗
    virtual void SetIdleDimmed(gint32 scale, MethodInvocation &invocation);

    virtual bool on_battery_setHandler(bool value) { return true; }
    virtual bool lid_is_present_setHandler(bool value) { return true; }
    virtual bool idle_dimmed_scale_setHandler(gint32 value);

    // 系统是否在使用电池供电
    virtual bool on_battery_get();
    // 是否存在笔记本盖子
    virtual bool lid_is_present_get();
    virtual gint32 idle_dimmed_scale_get();

private:
    void init();

    void on_battery_changed(bool on_battery);
    void on_lid_is_closed_changed(bool lid_is_closed);

    void on_settings_changed(const Glib::ustring &key);
    void on_brightness_changed(std::shared_ptr<PowerBacklightPercentage> backlight_device, int32_t brightness_value);

    void on_bus_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name);
    void on_name_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name);
    void on_name_lost(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name);

private:
    static PowerManager *instance_;

    PowerWrapperManager *wrapper_manager_;
    PowerBacklight *backlight_;
    std::shared_ptr<PowerUPower> upower_client_;

    Glib::RefPtr<Gio::Settings> power_settings_;

    // dbus
    uint32_t dbus_connect_id_;
    uint32_t object_register_id_;
};
}  // namespace Kiran