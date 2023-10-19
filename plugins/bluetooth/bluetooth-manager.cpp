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

#include "plugins/bluetooth/bluetooth-manager.h"

#include <glib-unix.h>

#include "lib/base/base.h"
#include "plugins/bluetooth/bluez.h"

namespace Kiran
{
#define BLUETOOTH_DBUS_NAME "com.kylinsec.Kiran.SessionDaemon.Bluetooth"
#define BLUETOOTH_OBJECT_PATH "/com/kylinsec/Kiran/SessionDaemon/Bluetooth"

BluetoothManager::BluetoothManager() : dbus_connect_id_(0),
                                       object_register_id_(0)
{
}

BluetoothManager::~BluetoothManager()
{
    if (this->dbus_connect_id_)
    {
        Gio::DBus::unown_name(this->dbus_connect_id_);
    }
}

BluetoothManager *BluetoothManager::instance_ = nullptr;

void BluetoothManager::global_init()
{
    instance_ = new BluetoothManager();
    instance_->init();
}

std::shared_ptr<BluetoothAdapter> BluetoothManager::get_adapter(const std::string &object_path)
{
    auto iter = this->adapters_.find(object_path);
    if (iter != this->adapters_.end())
    {
        return iter->second;
    }
    return nullptr;
}

std::shared_ptr<BluetoothAdapter> BluetoothManager::get_adapter_by_device(const std::string &object_path)
{
    for (auto &iter : this->adapters_)
    {
        if (iter.second->find_device(object_path))
        {
            return iter.second;
        }
    }
    return nullptr;
}

void BluetoothManager::GetAdapters(MethodInvocation &invocation)
{
    std::vector<Glib::ustring> adapters;
    for (auto &iter : this->adapters_)
    {
        adapters.push_back(iter.second->get_object_path());
    }
    invocation.ret(adapters);
}

void BluetoothManager::GetDevices(const Glib::DBusObjectPathString &adapter_object_path, MethodInvocation &invocation)
{
    std::vector<Glib::ustring> devices;
    auto adapter = this->get_adapter(adapter_object_path);
    if (!adapter)
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_BLUETOOTH_NOTFOUND_ADAPTOR);
    }

    for (auto &device : adapter->get_devices())
    {
        devices.push_back(device->get_object_path());
    }

    invocation.ret(devices);
}

void BluetoothManager::FeedPinCode(const Glib::DBusObjectPathString &device,
                                   bool accept,
                                   const Glib::ustring &pincode,
                                   MethodInvocation &invocation)
{
    KLOG_DEBUG_BLUETOOTH("Feed pin code,device is %s, accept is %d, pincode is %s.", device.c_str(), accept, pincode.c_str());
    this->agent_feeded_.emit(accept, pincode.raw());
    invocation.ret();
}

void BluetoothManager::FeedPasskey(const Glib::DBusObjectPathString &device,
                                   bool accept,
                                   guint32 passkey,
                                   MethodInvocation &invocation)
{
    KLOG_DEBUG_BLUETOOTH("Feed pass key,device is %s, accept is %d passkey is %u.", device.c_str(), accept, passkey);
    this->agent_feeded_.emit(accept, fmt::format("{0}", passkey));
    invocation.ret();
}

void BluetoothManager::Confirm(const Glib::DBusObjectPathString &device,
                               bool accept,
                               MethodInvocation &invocation)
{
    KLOG_DEBUG_BLUETOOTH("Confirm,device is %s, accept is %d.", device.c_str(), accept);
    this->agent_feeded_.emit(accept, std::string());
    invocation.ret();
}

void BluetoothManager::init()
{
    this->agent_ = std::make_shared<BluetoothAgent>(this);
    this->agent_->init();

    DBus::ObjectManagerProxy::createForBus(Gio::DBus::BUS_TYPE_SYSTEM,
                                           Gio::DBus::PROXY_FLAGS_NONE,
                                           BLUEZ_DBUS_NAME,
                                           BLUEZ_ROOT_OBJECT_PATH,
                                           sigc::mem_fun(this, &BluetoothManager::on_bluez_ready));

    this->dbus_connect_id_ = Gio::DBus::own_name(Gio::DBus::BUS_TYPE_SESSION,
                                                 BLUETOOTH_DBUS_NAME,
                                                 sigc::mem_fun(this, &BluetoothManager::on_bus_acquired),
                                                 sigc::mem_fun(this, &BluetoothManager::on_name_acquired),
                                                 sigc::mem_fun(this, &BluetoothManager::on_name_lost));
}

void BluetoothManager::on_bluez_ready(Glib::RefPtr<Gio::AsyncResult> &result)
{
    try
    {
        this->objects_proxy_ = DBus::ObjectManagerProxy::createForBusFinish(result);
    }
    catch (const Glib::Error &e)
    {
        KLOG_WARNING_BLUETOOTH("Cannot connect to %s: %s.", BLUEZ_ROOT_OBJECT_PATH, e.what().c_str());
        return;
    }

    this->objects_proxy_->InterfacesAdded_signal.connect(sigc::mem_fun(this, &BluetoothManager::on_interface_added));
    this->objects_proxy_->InterfacesRemoved_signal.connect(sigc::mem_fun(this, &BluetoothManager::on_interface_removed));

    this->load_objects();
}

void BluetoothManager::on_interface_added(Glib::DBusObjectPathString object_path,
                                          std::map<Glib::ustring, std::map<Glib::ustring, Glib::VariantBase>> interfaces)
{
    KLOG_DEBUG_BLUETOOTH(" Added interface object_path: %s.", object_path.c_str());

    if (interfaces.find(BLUEZ_ADAPTER_INTERFACE_NAME) != interfaces.end())
    {
        this->add_adapter(object_path);
    }

    if (interfaces.find(BLUEZ_DEVICE_INTERFACE_NAME) != interfaces.end())
    {
        this->add_device(object_path);
    }
}

void BluetoothManager::on_interface_removed(Glib::DBusObjectPathString object_path,
                                            std::vector<Glib::ustring> interfaces)
{
    KLOG_DEBUG_BLUETOOTH("Removed interface object_path: %s.", object_path.c_str());
    if (std::find(interfaces.begin(), interfaces.end(), BLUEZ_ADAPTER_INTERFACE_NAME) != interfaces.end())
    {
        this->remove_adapter(object_path);
    }

    if (std::find(interfaces.begin(), interfaces.end(), BLUEZ_DEVICE_INTERFACE_NAME) != interfaces.end())
    {
        this->remove_device(object_path);
    }
}

void BluetoothManager::load_objects()
{
    KLOG_DEBUG_BLUETOOTH("Load objects");
    auto objects = this->objects_proxy_->GetManagedObjects_sync();

    // Add adapters
    for (const auto &object : objects)
    {
        if (object.second.find(BLUEZ_ADAPTER_INTERFACE_NAME) != object.second.end())
        {
            this->add_adapter(object.first);
        }
    }

    // Add devices
    for (const auto &object : objects)
    {
        if (object.second.find(BLUEZ_DEVICE_INTERFACE_NAME) != object.second.end())
        {
            this->add_device(object.first);
        }
    }
}

void BluetoothManager::add_adapter(const std::string &object_path)
{
    KLOG_DEBUG_BLUETOOTH("Add adapter,object_path is %s.", object_path.c_str());

    auto adapter = std::make_shared<BluetoothAdapter>(object_path);
    auto iter = this->adapters_.emplace(object_path, adapter);
    if (!iter.second)
    {
        KLOG_WARNING_BLUETOOTH("Insert adapter %s failed.", object_path.c_str());
        return;
    }
    this->AdapterAdded_signal.emit(object_path);
}

void BluetoothManager::remove_adapter(const std::string &object_path)
{
    auto iter = this->adapters_.find(object_path);
    if (iter == this->adapters_.end())
    {
        KLOG_WARNING_BLUETOOTH("Not found adapter %s.", object_path.c_str());
        return;
    }
    this->adapters_.erase(iter);
    this->AdapterRemoved_signal.emit(object_path);
}

void BluetoothManager::add_device(const std::string &object_path)
{
    auto device = std::make_shared<BluetoothDevice>(object_path);
    auto adapter_object_path = device->get_adapter();
    auto adapter = this->get_adapter(adapter_object_path);
    if (!adapter)
    {
        KLOG_WARNING_BLUETOOTH("Not found adapter %s.", adapter_object_path.c_str());
    }
    else
    {
        adapter->add_device(device);
        this->DeviceAdded_signal.emit(object_path);
    }
}

void BluetoothManager::remove_device(const std::string &object_path)
{
    auto adapter = this->get_adapter_by_device(object_path);
    if (!adapter)
    {
        KLOG_WARNING_BLUETOOTH("Not found adapter for device %s.", object_path.c_str());
        return;
    }
    adapter->remove_device(object_path);
    this->DeviceRemove_signal.emit(object_path);
}

void BluetoothManager::on_bus_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name)
{
    if (!connect)
    {
        KLOG_WARNING_BLUETOOTH("Failed to connect dbus with %s", name.c_str());
        return;
    }
    try
    {
        this->object_register_id_ = this->register_object(connect, BLUETOOTH_OBJECT_PATH);
    }
    catch (const Glib::Error &e)
    {
        KLOG_WARNING_BLUETOOTH("Register object_path %s fail: %s.", BLUETOOTH_OBJECT_PATH, e.what().c_str());
    }
}

void BluetoothManager::on_name_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name)
{
    KLOG_DEBUG_BLUETOOTH("Success to register dbus name: %s", name.c_str());
}

void BluetoothManager::on_name_lost(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name)
{
    KLOG_WARNING_BLUETOOTH("Failed to register dbus name: %s", name.c_str());
}

}  // namespace Kiran