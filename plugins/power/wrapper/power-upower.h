/**
 * @file          /kiran-cc-daemon/plugins/power/wrapper/power-upower.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
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

    /* 混合电源设备。部分机器可能存在多个电池设备，当使用电池供电时，系统供电状态应该依赖所有电池设备的电量情况，而不是单个电池的电量，
       因此，如果想获取系统电池整体使用情况，需要使用混合电源设备*/
    std::shared_ptr<PowerUPowerDevice> get_display_device() { return this->display_device_; };
    PowerUPowerDeviceVec get_devices();

    sigc::signal<void, bool> &signal_on_battery_changed() { return this->on_battery_changed_; };
    sigc::signal<void, bool> &signal_lid_is_closed_changed() { return this->lid_is_closed_changed_; };

    using DeviceChangedSignalType = sigc::signal<void, std::shared_ptr<PowerUPowerDevice>, UPowerDeviceEvent>;
    // 设备状态发生变化
    DeviceChangedSignalType &signal_device_status_changed() { return this->device_status_changed_; };

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
                                 std::shared_ptr<PowerUPowerDevice> device);

private:
    Glib::RefPtr<Gio::DBus::Proxy> upower_proxy_;

    bool on_battery_;
    bool lid_is_closed_;
    bool lid_is_present_;

    std::shared_ptr<PowerUPowerDevice> display_device_;
    std::map<Glib::DBusObjectPathString, std::shared_ptr<PowerUPowerDevice>> devices_;

    sigc::signal<void, bool> on_battery_changed_;
    sigc::signal<void, bool> lid_is_closed_changed_;
    DeviceChangedSignalType device_status_changed_;
};
}  // namespace Kiran