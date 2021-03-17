/**
 * @file          /kiran-cc-daemon/plugins/power/wrapper/power-upower-device.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#pragma once

#include <upower.h>

#include "lib/base/base.h"

namespace Kiran
{
struct UPowerDeviceProps
{
    std::string native_path;
    std::string vendor;
    std::string model;
    std::string serial;
    uint64_t update_time;
    uint32_t type;
    bool power_supply;
    bool has_history;
    bool has_statistics;
    bool online;
    double energy;
    double energy_empty;
    double energy_full;
    double energy_full_design;
    double energy_rate;
    double voltage;
    double luminosity;
    int64_t time_to_empty;
    int64_t time_to_full;
    double percentage;
    double temperature;
    bool is_present;
    uint32_t state;
    bool is_rechargeable;
    double capacity;
    uint32_t technology;
    uint32_t warning_level;
    uint32_t battery_level;
    std::string icon_name;
};

enum UPowerDeviceEvent
{
    // 断开充电
    UPOWER_DEVICE_EVENT_DISCHARGING,
    // 电量充满
    UPOWER_DEVICE_EVENT_FULLY_CHARGED,
    // 电量过低
    UPOWER_DEVICE_EVENT_CHARGE_LOW,
    // 电量过低
    UPOWER_DEVICE_EVENT_CHARGE_CRITICAL,
    // 电量过低
    UPOWER_DEVICE_EVENT_CHARGE_ACTION,

};

class PowerUPowerDevice
{
public:
    PowerUPowerDevice(const Glib::DBusObjectPathString& object_path);
    virtual ~PowerUPowerDevice();

    // 获取设备所有属性
    const UPowerDeviceProps& get_props() { return this->props_; };

    // 获取设备的dbus object path
    const Glib::DBusObjectPathString& get_object_path() { return this->object_path_; };

    // 获取设备类型的翻译，number>1时翻译为复数
    std::string get_type_translation(uint32_t number);

    // 属性发生变化，参数分别为返回值、旧的属性和新的属性
    sigc::signal<void, const UPowerDeviceProps&, const UPowerDeviceProps&>& signal_props_changed() { return this->props_changed_; };

private:
    void load_device_props();

    Glib::ustring get_property_string(const std::string& property_name);
    uint64_t get_property_uint64(const std::string& property_name);
    int64_t get_property_int64(const std::string& property_name);
    uint32_t get_property_uint(const std::string& property_name);
    int32_t get_property_int(const std::string& property_name);
    bool get_property_bool(const std::string& property_name);
    double get_property_double(const std::string& property_name);

    void update_properties(const Gio::DBus::Proxy::MapChangedProperties& changed_properties);

    void on_properties_changed(const Gio::DBus::Proxy::MapChangedProperties& changed_properties,
                               const std::vector<Glib::ustring>& invalidated_properties);

private:
    Glib::RefPtr<Gio::DBus::Proxy> upower_device_proxy_;

    Glib::DBusObjectPathString object_path_;

    UPowerDeviceProps props_;
    sigc::signal<void, const UPowerDeviceProps&, const UPowerDeviceProps&> props_changed_;
};

using PowerUPowerDeviceVec = std::vector<std::shared_ptr<PowerUPowerDevice>>;
}  // namespace Kiran