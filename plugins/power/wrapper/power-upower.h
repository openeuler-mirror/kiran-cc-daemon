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

#include "lib/base/base.h"
#include "plugins/power/wrapper/power-upower-device.h"

namespace Kiran
{
class PowerUPower
{
public:
    PowerUPower();
    virtual ~PowerUPower();

    void init();

    // 是否使用电池供电
    bool get_on_battery() { return this->on_battery_; };
    // 笔记本盖子是否关闭
    bool get_lid_is_closed() { return this->lid_is_closed_; };
    // 是否有笔记本盖子
    bool get_lid_is_present() { return this->lid_is_present_; };

    /* 混合电池设备。部分机器可能存在多个电池设备，当使用电池供电时，系统供电状态应该依赖所有电池设备的电量情况，而不是单个电池的电量，
       因此，如果想获取系统电池整体使用情况，需要使用混合电源设备*/
    std::shared_ptr<PowerUPowerDevice> get_display_device() { return this->display_device_; };
    PowerUPowerDeviceVec get_devices();
    std::shared_ptr<PowerUPowerDevice> get_device(const Glib::DBusObjectPathString &object_path) { return MapHelper::get_value(this->devices_, object_path); };

    sigc::signal<void, bool> &signal_on_battery_changed() { return this->on_battery_changed_; };
    sigc::signal<void, bool> &signal_lid_is_closed_changed() { return this->lid_is_closed_changed_; };

    using DeviceStatusChangedSignalType = sigc::signal<void, std::shared_ptr<PowerUPowerDevice>, UPowerDeviceEvent>;
    using DevicePropsChangedSignalType = sigc::signal<void, std::shared_ptr<PowerUPowerDevice>, const UPowerDeviceProps &, const UPowerDeviceProps &>;

    // 设备状态发生变化
    DeviceStatusChangedSignalType &signal_device_status_changed() { return this->device_status_changed_; };
    // 设备属性发生变化，参数分别为返回值、设备对象，旧的属性和新的属性
    DevicePropsChangedSignalType &signal_device_props_changed() { return this->device_props_changed_; };

private:
    Glib::DBusObjectPathString get_display_device_object_path();
    std::vector<Glib::DBusObjectPathString> get_devices_object_path();

    bool add_upower_device(const Glib::DBusObjectPathString &object_path);
    bool del_upower_device(const Glib::DBusObjectPathString &object_path);

    void on_properties_changed(const Gio::DBus::Proxy::MapChangedProperties &changed_properties,
                               const std::vector<Glib::ustring> &invalidated_properties);

    void on_upower_signal(const Glib::ustring &sender_name,
                          const Glib::ustring &signal_name,
                          const Glib::VariantContainerBase &parameters);

    void on_device_props_changed(const UPowerDeviceProps &old_props,
                                 const UPowerDeviceProps &new_props,
                                 Glib::DBusObjectPathString device_object_path);

private:
    Glib::RefPtr<Gio::DBus::Proxy> upower_proxy_;

    bool on_battery_;
    bool lid_is_closed_;
    bool lid_is_present_;

    std::shared_ptr<PowerUPowerDevice> display_device_;
    std::map<Glib::DBusObjectPathString, std::shared_ptr<PowerUPowerDevice>> devices_;

    sigc::signal<void, bool> on_battery_changed_;
    sigc::signal<void, bool> lid_is_closed_changed_;

    DeviceStatusChangedSignalType device_status_changed_;
    DevicePropsChangedSignalType device_props_changed_;
};
}  // namespace Kiran