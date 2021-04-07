/**
 * @file          /kiran-cc-daemon/plugins/power/wrapper/power-upower.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#include "plugins/power/wrapper/power-upower.h"

namespace Kiran
{
#define UPOWER_DBUS_NAME "org.freedesktop.UPower"
#define UPOWER_DBUS_OBJECT "/org/freedesktop/UPower"
#define UPOWER_DBUS_INTERFACE "org.freedesktop.UPower"
#define UPOWER_DBUS_PROP_ON_BATTERY "OnBattery"
#define UPOWER_DBUS_PROP_LID_IS_CLOSED "LidIsClosed"
#define UPOWER_DBUS_PROP_LID_IS_PRESENT "LidIsPresent"

PowerUPower::PowerUPower() : on_battery_(false),
                             lid_is_closed_(false),
                             lid_is_present_(false)
{
}

PowerUPower::~PowerUPower()
{
}

void PowerUPower::init()
{
    try
    {
        this->upower_proxy_ = Gio::DBus::Proxy::create_for_bus_sync(Gio::DBus::BUS_TYPE_SYSTEM,
                                                                    UPOWER_DBUS_NAME,
                                                                    UPOWER_DBUS_OBJECT,
                                                                    UPOWER_DBUS_INTERFACE);
    }
    catch (const Glib::Error &e)
    {
        LOG_WARNING("%s", e.what());
        return;
    }

    Glib::VariantBase property;

    try
    {
        this->upower_proxy_->get_cached_property(property, UPOWER_DBUS_PROP_ON_BATTERY);
        this->on_battery_ = Glib::VariantBase::cast_dynamic<Glib::Variant<bool>>(property).get();

        this->upower_proxy_->get_cached_property(property, UPOWER_DBUS_PROP_LID_IS_CLOSED);
        this->lid_is_closed_ = Glib::VariantBase::cast_dynamic<Glib::Variant<bool>>(property).get();

        this->upower_proxy_->get_cached_property(property, UPOWER_DBUS_PROP_LID_IS_PRESENT);
        this->lid_is_present_ = Glib::VariantBase::cast_dynamic<Glib::Variant<bool>>(property).get();
    }
    catch (const std::exception &e)
    {
        LOG_WARNING("%s", e.what());
    }

    auto display_device_object_path = this->get_display_device_object_path();
    this->display_device_ = std::make_shared<PowerUPowerDevice>(display_device_object_path);
    this->display_device_->signal_props_changed().connect(sigc::bind(sigc::mem_fun(this, &PowerUPower::on_device_props_changed),
                                                                     this->display_device_));

    auto devices_object_path = this->get_devices_object_path();
    for (auto &o : devices_object_path)
    {
        this->add_upower_device(o);
    }

    this->upower_proxy_->signal_properties_changed().connect(sigc::mem_fun(this, &PowerUPower::on_properties_changed));
    this->upower_proxy_->signal_signal().connect(sigc::mem_fun(this, &PowerUPower::on_upower_signal));
}

PowerUPowerDeviceVec PowerUPower::get_devices()
{
    PowerUPowerDeviceVec devices;
    for (auto &iter : this->devices_)
    {
        devices.push_back(iter.second);
    }
    return devices;
}

Glib::DBusObjectPathString PowerUPower::get_display_device_object_path()
{
    RETURN_VAL_IF_FALSE(this->upower_proxy_, Glib::DBusObjectPathString());

    try
    {
        auto retval = this->upower_proxy_->call_sync("GetDisplayDevice", Glib::VariantContainerBase());
        auto v1 = retval.get_child(0);
        return Glib::VariantBase::cast_dynamic<Glib::Variant<Glib::DBusObjectPathString>>(v1).get().raw();
    }
    catch (const Glib::Error &e)
    {
        LOG_WARNING("%s", e.what().c_str());
    }
    catch (const std::exception &e)
    {
        LOG_WARNING("%s", e.what());
    }
    return Glib::DBusObjectPathString();
}

std::vector<Glib::DBusObjectPathString> PowerUPower::get_devices_object_path()
{
    RETURN_VAL_IF_FALSE(this->upower_proxy_, std::vector<Glib::DBusObjectPathString>());

    try
    {
        auto retval = this->upower_proxy_->call_sync("EnumerateDevices", Glib::VariantContainerBase());
        auto v1 = retval.get_child(0);
        return Glib::VariantBase::cast_dynamic<Glib::Variant<std::vector<Glib::DBusObjectPathString>>>(v1).get();
    }
    catch (const Glib::Error &e)
    {
        LOG_WARNING("%s", e.what().c_str());
    }
    catch (const std::exception &e)
    {
        LOG_WARNING("%s", e.what());
    }
    return std::vector<Glib::DBusObjectPathString>();
}

bool PowerUPower::add_upower_device(const Glib::DBusObjectPathString &object_path)
{
    auto device = std::make_shared<PowerUPowerDevice>(object_path);
    auto iter = this->devices_.emplace(object_path, device);
    if (!iter.second)
    {
        LOG_WARNING("The upwer device %s already exists.", object_path.c_str());
        return false;
    }
    device->signal_props_changed().connect(sigc::bind(sigc::mem_fun(this, &PowerUPower::on_device_props_changed), device));
    return true;
}

bool PowerUPower::del_upower_device(const Glib::DBusObjectPathString &object_path)
{
    auto iter = this->devices_.find(object_path);
    if (iter == this->devices_.end())
    {
        LOG_WARNING("The upower device %s doesn't exist.", object_path.c_str());
        return false;
    }
    this->devices_.erase(iter);
    return true;
}

void PowerUPower::on_properties_changed(const Gio::DBus::Proxy::MapChangedProperties &changed_properties,
                                        const std::vector<Glib::ustring> &invalidated_properties)
{
    try
    {
        for (auto &iter : changed_properties)
        {
            switch (shash(iter.first.c_str()))
            {
            case CONNECT(UPOWER_DBUS_PROP_ON_BATTERY, _hash):
                this->on_battery_ = Glib::VariantBase::cast_dynamic<Glib::Variant<bool>>(iter.second).get();
                this->on_battery_changed_.emit(this->on_battery_);
                break;
            case CONNECT(UPOWER_DBUS_PROP_LID_IS_CLOSED, _hash):
                this->lid_is_closed_ = Glib::VariantBase::cast_dynamic<Glib::Variant<bool>>(iter.second).get();
                this->lid_is_closed_changed_.emit(this->lid_is_closed_);
                break;
            case CONNECT(UPOWER_DBUS_PROP_LID_IS_PRESENT, _hash):
                this->lid_is_present_ = Glib::VariantBase::cast_dynamic<Glib::Variant<bool>>(iter.second).get();
                break;
            }
        }
    }
    catch (const std::exception &e)
    {
        LOG_WARNING("%s", e.what());
    }

    return;
}

void PowerUPower::on_upower_signal(const Glib::ustring &sender_name,
                                   const Glib::ustring &signal_name,
                                   const Glib::VariantContainerBase &parameters)
{
    SETTINGS_PROFILE("sender_name: %s, signal_name: %s.", sender_name.c_str(), signal_name.c_str());

    switch (shash(signal_name.c_str()))
    {
    case "DeviceAdd"_hash:
    {
        auto o = Glib::VariantBase::cast_dynamic<Glib::Variant<Glib::DBusObjectPathString>>(parameters).get();
        this->add_upower_device(o);
        break;
    }
    case "DeviceRemoved"_hash:
    {
        auto o = Glib::VariantBase::cast_dynamic<Glib::Variant<Glib::DBusObjectPathString>>(parameters).get();
        this->del_upower_device(o);
        break;
    }
    default:
        break;
    }
}

void PowerUPower::on_device_props_changed(const UPowerDeviceProps &old_props,
                                          const UPowerDeviceProps &new_props,
                                          std::shared_ptr<PowerUPowerDevice> device)
{
    // 不处理单个电池设备的状态信息，电池设备的状态信息以混合设备的为准
    if (new_props.type == UP_DEVICE_KIND_BATTERY &&
        device->get_object_path() != this->display_device_->get_object_path())
    {
        return;
    }

    if (old_props.state != new_props.state)
    {
        switch (new_props.state)
        {
        case UP_DEVICE_STATE_DISCHARGING:
            this->device_status_changed_.emit(device, UPowerDeviceEvent::UPOWER_DEVICE_EVENT_DISCHARGING);
            break;
        case UP_DEVICE_STATE_FULLY_CHARGED:
            this->device_status_changed_.emit(device, UPowerDeviceEvent::UPOWER_DEVICE_EVENT_FULLY_CHARGED);
            break;
        default:
            break;
        }
    }

    if (old_props.warning_level != new_props.warning_level)
    {
        switch (new_props.warning_level)
        {
        case UP_DEVICE_LEVEL_LOW:
            this->device_status_changed_.emit(device, UPowerDeviceEvent::UPOWER_DEVICE_EVENT_CHARGE_LOW);
            break;
        case UP_DEVICE_LEVEL_CRITICAL:
            this->device_status_changed_.emit(device, UPowerDeviceEvent::UPOWER_DEVICE_EVENT_CHARGE_CRITICAL);
            break;
        case UP_DEVICE_LEVEL_ACTION:
            this->device_status_changed_.emit(device, UPowerDeviceEvent::UPOWER_DEVICE_EVENT_CHARGE_ACTION);
        default:
            break;
        }
    }

    this->device_props_changed_.emit(device, old_props, new_props);
}

}  // namespace Kiran