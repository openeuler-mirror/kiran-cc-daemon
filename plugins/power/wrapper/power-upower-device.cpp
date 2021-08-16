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

#include "plugins/power/wrapper/power-upower-device.h"

#include <glib/gi18n.h>

namespace Kiran
{
#define UPOWER_DEVICE_DBUS_NAME "org.freedesktop.UPower"
#define UPOWER_DEVICE_DBUS_INTERFACE "org.freedesktop.UPower.Device"

#define UPOWER_DEVICE_DBUS_PROP_NATIVE_PATH "NativePath"
#define UPOWER_DEVICE_DBUS_PROP_VENDOR "Vendor"
#define UPOWER_DEVICE_DBUS_PROP_MODEL "Model"
#define UPOWER_DEVICE_DBUS_PROP_SERIAL "Serial"
#define UPOWER_DEVICE_DBUS_PROP_UPDATE_TIME "UpdateTime"
#define UPOWER_DEVICE_DBUS_PROP_TYPE "Type"
#define UPOWER_DEVICE_DBUS_PROP_POWER_SUPPLY "PowerSupply"
#define UPOWER_DEVICE_DBUS_PROP_HAS_HISTORY "HasHistory"
#define UPOWER_DEVICE_DBUS_PROP_HAS_STATISTICS "HasStatistics"
#define UPOWER_DEVICE_DBUS_PROP_ONLINE "Online"
#define UPOWER_DEVICE_DBUS_PROP_ENERGY "Energy"
#define UPOWER_DEVICE_DBUS_PROP_ENERGY_EMPTY "EnergyEmpty"
#define UPOWER_DEVICE_DBUS_PROP_ENERGY_FULL "EnergyFull"
#define UPOWER_DEVICE_DBUS_PROP_ENERGY_FULL_DESIGN "EnergyFullDesign"
#define UPOWER_DEVICE_DBUS_PROP_ENERGY_RATE "EnergyRate"
#define UPOWER_DEVICE_DBUS_PROP_VOLTAGE "Voltage"
#define UPOWER_DEVICE_DBUS_PROP_LUMINOSITY "Luminosity"
#define UPOWER_DEVICE_DBUS_PROP_TIME_TO_EMPTY "TimeToEmpty"
#define UPOWER_DEVICE_DBUS_PROP_TIME_TO_FULL "TimeToFull"
#define UPOWER_DEVICE_DBUS_PROP_PERCENTAGE "Percentage"
#define UPOWER_DEVICE_DBUS_PROP_TEMPERATURE "Temperature"
#define UPOWER_DEVICE_DBUS_PROP_IS_PRESENT "IsPresent"
#define UPOWER_DEVICE_DBUS_PROP_STATE "State"
#define UPOWER_DEVICE_DBUS_PROP_IS_RECHARGEABLE "IsRechargeable"
#define UPOWER_DEVICE_DBUS_PROP_CAPACITY "Capacity"
#define UPOWER_DEVICE_DBUS_PROP_TECHNOLOGY "Technology"
#define UPOWER_DEVICE_DBUS_PROP_WARNING_LEVEL "WarningLevel"
#define UPOWER_DEVICE_DBUS_PROP_BATTERY_LEVEL "BatteryLevel"
#define UPOWER_DEVICE_DBUS_PROP_ICON_NAME "IconName"

PowerUPowerDevice::PowerUPowerDevice(const Glib::DBusObjectPathString& object_path) : object_path_(object_path)
{
    KLOG_DEBUG("object path: %s", object_path.c_str());

    try
    {
        this->upower_device_proxy_ = Gio::DBus::Proxy::create_for_bus_sync(Gio::DBus::BUS_TYPE_SYSTEM,
                                                                           UPOWER_DEVICE_DBUS_NAME,
                                                                           this->object_path_,
                                                                           UPOWER_DEVICE_DBUS_INTERFACE);
    }
    catch (const Glib::Error& e)
    {
        KLOG_WARNING("%s", e.what().c_str());
        return;
    }

    this->load_device_props();
    this->upower_device_proxy_->signal_properties_changed().connect(sigc::mem_fun(this, &PowerUPowerDevice::on_properties_changed));
}

PowerUPowerDevice::~PowerUPowerDevice()
{
}

std::string PowerUPowerDevice::get_type_translation(uint32_t number)
{
    switch (this->props_.type)
    {
    // 电源
    case UP_DEVICE_KIND_LINE_POWER:
        return POINTER_TO_STRING(ngettext("AC adapter", "AC adapters", number));
    // 笔记本电池
    case UP_DEVICE_KIND_BATTERY:
        return POINTER_TO_STRING(ngettext("Laptop battery", "Laptop batteries", number));
    // 备用电池
    case UP_DEVICE_KIND_UPS:
        return POINTER_TO_STRING(ngettext("UPS", "UPSs", number));
    // 显示器
    case UP_DEVICE_KIND_MONITOR:
        return POINTER_TO_STRING(ngettext("Monitor", "Monitors", number));
    // 鼠标
    case UP_DEVICE_KIND_MOUSE:
        return POINTER_TO_STRING(ngettext("Mouse", "Mice", number));
    // 键盘
    case UP_DEVICE_KIND_KEYBOARD:
        return POINTER_TO_STRING(ngettext("Keyboard", "Keyboards", number));
    // 个人数码助理
    case UP_DEVICE_KIND_PDA:
        return POINTER_TO_STRING(ngettext("PDA", "PDAs", number));
    // 手机
    case UP_DEVICE_KIND_PHONE:
        return POINTER_TO_STRING(ngettext("Cell phone", "Cell phones", number));
    // 媒体播放器
    case UP_DEVICE_KIND_MEDIA_PLAYER:
        return POINTER_TO_STRING(ngettext("Media player", "Media players", number));
    // 平板电脑
    case UP_DEVICE_KIND_TABLET:
        return POINTER_TO_STRING(ngettext("Tablet", "Tablets", number));
    // 计算机(平板电脑)
    case UP_DEVICE_KIND_COMPUTER:
        return POINTER_TO_STRING(ngettext("Computer", "Computers", number));
    default:
        KLOG_WARNING("Unknown type: %d", this->props_.type);
        return POINTER_TO_STRING(ngettext("Unknown", "Unknown", number));
    }
    return std::string();
}

void PowerUPowerDevice::load_device_props()
{
    this->props_.native_path = this->get_property_string(UPOWER_DEVICE_DBUS_PROP_NATIVE_PATH);
    this->props_.vendor = this->get_property_string(UPOWER_DEVICE_DBUS_PROP_VENDOR);
    this->props_.model = this->get_property_string(UPOWER_DEVICE_DBUS_PROP_NATIVE_PATH);
    this->props_.serial = this->get_property_string(UPOWER_DEVICE_DBUS_PROP_SERIAL);
    this->props_.update_time = this->get_property_uint64(UPOWER_DEVICE_DBUS_PROP_UPDATE_TIME);
    this->props_.type = this->get_property_uint(UPOWER_DEVICE_DBUS_PROP_TYPE);
    this->props_.power_supply = this->get_property_bool(UPOWER_DEVICE_DBUS_PROP_POWER_SUPPLY);
    this->props_.has_history = this->get_property_bool(UPOWER_DEVICE_DBUS_PROP_HAS_HISTORY);
    this->props_.has_statistics = this->get_property_bool(UPOWER_DEVICE_DBUS_PROP_HAS_STATISTICS);
    this->props_.online = this->get_property_bool(UPOWER_DEVICE_DBUS_PROP_ONLINE);
    this->props_.energy = this->get_property_double(UPOWER_DEVICE_DBUS_PROP_ENERGY);
    this->props_.energy_empty = this->get_property_double(UPOWER_DEVICE_DBUS_PROP_ENERGY_EMPTY);
    this->props_.energy_full = this->get_property_double(UPOWER_DEVICE_DBUS_PROP_ENERGY_FULL);
    this->props_.energy_full_design = this->get_property_double(UPOWER_DEVICE_DBUS_PROP_ENERGY_FULL_DESIGN);
    this->props_.energy_rate = this->get_property_double(UPOWER_DEVICE_DBUS_PROP_ENERGY_RATE);
    this->props_.voltage = this->get_property_double(UPOWER_DEVICE_DBUS_PROP_VOLTAGE);
    this->props_.luminosity = this->get_property_double(UPOWER_DEVICE_DBUS_PROP_LUMINOSITY);
    this->props_.time_to_empty = this->get_property_int64(UPOWER_DEVICE_DBUS_PROP_TIME_TO_EMPTY);
    this->props_.time_to_full = this->get_property_int64(UPOWER_DEVICE_DBUS_PROP_TIME_TO_FULL);
    this->props_.percentage = this->get_property_double(UPOWER_DEVICE_DBUS_PROP_PERCENTAGE);
    this->props_.temperature = this->get_property_double(UPOWER_DEVICE_DBUS_PROP_TEMPERATURE);
    this->props_.is_present = this->get_property_bool(UPOWER_DEVICE_DBUS_PROP_IS_PRESENT);
    this->props_.state = this->get_property_uint(UPOWER_DEVICE_DBUS_PROP_STATE);
    this->props_.is_rechargeable = this->get_property_bool(UPOWER_DEVICE_DBUS_PROP_IS_RECHARGEABLE);
    this->props_.capacity = this->get_property_double(UPOWER_DEVICE_DBUS_PROP_CAPACITY);
    this->props_.technology = this->get_property_uint(UPOWER_DEVICE_DBUS_PROP_TECHNOLOGY);
    this->props_.warning_level = this->get_property_uint(UPOWER_DEVICE_DBUS_PROP_WARNING_LEVEL);
    this->props_.battery_level = this->get_property_uint(UPOWER_DEVICE_DBUS_PROP_BATTERY_LEVEL);
    this->props_.icon_name = this->get_property_string(UPOWER_DEVICE_DBUS_PROP_ICON_NAME);
    KLOG_DEBUG("icon name: %s.", this->props_.icon_name.c_str());
}

#define GET_PROPERTY_FUNC(type, rettype)                                                    \
    rettype PowerUPowerDevice::get_property_##type(const std::string& property_name)        \
    {                                                                                       \
        RETURN_VAL_IF_FALSE(this->upower_device_proxy_, rettype());                         \
                                                                                            \
        try                                                                                 \
        {                                                                                   \
            Glib::VariantBase property;                                                     \
            this->upower_device_proxy_->get_cached_property(property, property_name);       \
            RETURN_VAL_IF_TRUE(property.gobj() == NULL, rettype());                         \
            return Glib::VariantBase::cast_dynamic<Glib::Variant<rettype>>(property).get(); \
        }                                                                                   \
        catch (const std::exception& e)                                                     \
        {                                                                                   \
            KLOG_WARNING("%s", e.what());                                                   \
        }                                                                                   \
        return rettype();                                                                   \
    }

GET_PROPERTY_FUNC(string, Glib::ustring);
GET_PROPERTY_FUNC(uint64, uint64_t);
GET_PROPERTY_FUNC(int64, int64_t);
GET_PROPERTY_FUNC(uint, uint32_t);
GET_PROPERTY_FUNC(int, int32_t);
GET_PROPERTY_FUNC(bool, bool);
GET_PROPERTY_FUNC(double, double);

void PowerUPowerDevice::update_properties(const Gio::DBus::Proxy::MapChangedProperties& changed_properties)
{
    for (auto& iter : changed_properties)
    {
        switch (shash(iter.first.c_str()))
        {
        case CONNECT(UPOWER_DEVICE_DBUS_PROP_NATIVE_PATH, _hash):
            this->props_.native_path = this->get_property_string(UPOWER_DEVICE_DBUS_PROP_NATIVE_PATH);
            break;
        case CONNECT(UPOWER_DEVICE_DBUS_PROP_VENDOR, _hash):
            this->props_.vendor = this->get_property_string(UPOWER_DEVICE_DBUS_PROP_VENDOR);
            break;
        case CONNECT(UPOWER_DEVICE_DBUS_PROP_MODEL, _hash):
            this->props_.model = this->get_property_string(UPOWER_DEVICE_DBUS_PROP_NATIVE_PATH);
            break;
        case CONNECT(UPOWER_DEVICE_DBUS_PROP_SERIAL, _hash):
            this->props_.serial = this->get_property_string(UPOWER_DEVICE_DBUS_PROP_SERIAL);
            break;
        case CONNECT(UPOWER_DEVICE_DBUS_PROP_UPDATE_TIME, _hash):
            this->props_.update_time = this->get_property_uint64(UPOWER_DEVICE_DBUS_PROP_UPDATE_TIME);
            break;
        case CONNECT(UPOWER_DEVICE_DBUS_PROP_TYPE, _hash):
            this->props_.type = this->get_property_uint(UPOWER_DEVICE_DBUS_PROP_TYPE);
            break;
        case CONNECT(UPOWER_DEVICE_DBUS_PROP_POWER_SUPPLY, _hash):
            this->props_.power_supply = this->get_property_bool(UPOWER_DEVICE_DBUS_PROP_POWER_SUPPLY);
            break;
        case CONNECT(UPOWER_DEVICE_DBUS_PROP_HAS_HISTORY, _hash):
            this->props_.has_history = this->get_property_bool(UPOWER_DEVICE_DBUS_PROP_HAS_HISTORY);
            break;
        case CONNECT(UPOWER_DEVICE_DBUS_PROP_HAS_STATISTICS, _hash):
            this->props_.has_statistics = this->get_property_bool(UPOWER_DEVICE_DBUS_PROP_HAS_STATISTICS);
            break;
        case CONNECT(UPOWER_DEVICE_DBUS_PROP_ONLINE, _hash):
            this->props_.online = this->get_property_bool(UPOWER_DEVICE_DBUS_PROP_ONLINE);
            break;
        case CONNECT(UPOWER_DEVICE_DBUS_PROP_ENERGY, _hash):
            this->props_.energy = this->get_property_double(UPOWER_DEVICE_DBUS_PROP_ENERGY);
            break;
        case CONNECT(UPOWER_DEVICE_DBUS_PROP_ENERGY_EMPTY, _hash):
            this->props_.energy_empty = this->get_property_double(UPOWER_DEVICE_DBUS_PROP_ENERGY_EMPTY);
            break;
        case CONNECT(UPOWER_DEVICE_DBUS_PROP_ENERGY_FULL, _hash):
            this->props_.energy_full = this->get_property_double(UPOWER_DEVICE_DBUS_PROP_ENERGY_FULL);
            break;
        case CONNECT(UPOWER_DEVICE_DBUS_PROP_ENERGY_FULL_DESIGN, _hash):
            this->props_.energy_full_design = this->get_property_double(UPOWER_DEVICE_DBUS_PROP_ENERGY_FULL_DESIGN);
            break;
        case CONNECT(UPOWER_DEVICE_DBUS_PROP_ENERGY_RATE, _hash):
            this->props_.energy_rate = this->get_property_double(UPOWER_DEVICE_DBUS_PROP_ENERGY_RATE);
            break;
        case CONNECT(UPOWER_DEVICE_DBUS_PROP_VOLTAGE, _hash):
            this->props_.voltage = this->get_property_double(UPOWER_DEVICE_DBUS_PROP_VOLTAGE);
            break;
        case CONNECT(UPOWER_DEVICE_DBUS_PROP_LUMINOSITY, _hash):
            this->props_.luminosity = this->get_property_double(UPOWER_DEVICE_DBUS_PROP_LUMINOSITY);
            break;
        case CONNECT(UPOWER_DEVICE_DBUS_PROP_TIME_TO_EMPTY, _hash):
            this->props_.time_to_empty = this->get_property_int64(UPOWER_DEVICE_DBUS_PROP_TIME_TO_EMPTY);
            break;
        case CONNECT(UPOWER_DEVICE_DBUS_PROP_TIME_TO_FULL, _hash):
            this->props_.time_to_full = this->get_property_int64(UPOWER_DEVICE_DBUS_PROP_TIME_TO_FULL);
            break;
        case CONNECT(UPOWER_DEVICE_DBUS_PROP_PERCENTAGE, _hash):
            this->props_.percentage = this->get_property_double(UPOWER_DEVICE_DBUS_PROP_PERCENTAGE);
            break;
        case CONNECT(UPOWER_DEVICE_DBUS_PROP_TEMPERATURE, _hash):
            this->props_.temperature = this->get_property_double(UPOWER_DEVICE_DBUS_PROP_TEMPERATURE);
            break;
        case CONNECT(UPOWER_DEVICE_DBUS_PROP_IS_PRESENT, _hash):
            this->props_.is_present = this->get_property_bool(UPOWER_DEVICE_DBUS_PROP_IS_PRESENT);
            break;
        case CONNECT(UPOWER_DEVICE_DBUS_PROP_STATE, _hash):
            this->props_.state = this->get_property_uint(UPOWER_DEVICE_DBUS_PROP_STATE);
            break;
        case CONNECT(UPOWER_DEVICE_DBUS_PROP_IS_RECHARGEABLE, _hash):
            this->props_.is_rechargeable = this->get_property_bool(UPOWER_DEVICE_DBUS_PROP_IS_RECHARGEABLE);
            break;
        case CONNECT(UPOWER_DEVICE_DBUS_PROP_CAPACITY, _hash):
            this->props_.capacity = this->get_property_double(UPOWER_DEVICE_DBUS_PROP_CAPACITY);
            break;
        case CONNECT(UPOWER_DEVICE_DBUS_PROP_TECHNOLOGY, _hash):
            this->props_.technology = this->get_property_uint(UPOWER_DEVICE_DBUS_PROP_TECHNOLOGY);
            break;
        case CONNECT(UPOWER_DEVICE_DBUS_PROP_WARNING_LEVEL, _hash):
            this->props_.warning_level = this->get_property_uint(UPOWER_DEVICE_DBUS_PROP_WARNING_LEVEL);
            break;
        case CONNECT(UPOWER_DEVICE_DBUS_PROP_BATTERY_LEVEL, _hash):
            this->props_.battery_level = this->get_property_uint(UPOWER_DEVICE_DBUS_PROP_BATTERY_LEVEL);
            break;
        case CONNECT(UPOWER_DEVICE_DBUS_PROP_ICON_NAME, _hash):
            this->props_.icon_name = this->get_property_string(UPOWER_DEVICE_DBUS_PROP_ICON_NAME);
            break;
        default:
            break;
        }
    }
}

std::string PowerUPowerDevice::kind2str(UpDeviceKind type_enum)
{
    switch (type_enum)
    {
    case UP_DEVICE_KIND_LINE_POWER:
        return "line-power";
    case UP_DEVICE_KIND_BATTERY:
        return "battery";
    case UP_DEVICE_KIND_UPS:
        return "ups";
    case UP_DEVICE_KIND_MONITOR:
        return "monitor";
    case UP_DEVICE_KIND_MOUSE:
        return "mouse";
    case UP_DEVICE_KIND_KEYBOARD:
        return "keyboard";
    case UP_DEVICE_KIND_PDA:
        return "pda";
    case UP_DEVICE_KIND_PHONE:
        return "phone";
    case UP_DEVICE_KIND_MEDIA_PLAYER:
        return "media-player";
    case UP_DEVICE_KIND_TABLET:
        return "tablet";
    case UP_DEVICE_KIND_COMPUTER:
        return "computer";
    case UP_DEVICE_KIND_GAMING_INPUT:
        return "gaming-input";
    default:
        return "unknown";
    }
    return "unknown";
}

void PowerUPowerDevice::on_properties_changed(const Gio::DBus::Proxy::MapChangedProperties& changed_properties,
                                              const std::vector<Glib::ustring>& invalidated_properties)
{
    auto old_props = this->props_;

    this->update_properties(changed_properties);
    this->props_changed_.emit(old_props, this->props_);
}
}  // namespace Kiran