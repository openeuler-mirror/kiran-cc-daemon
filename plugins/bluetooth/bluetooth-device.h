/*
 * @Author       : tangjie02
 * @Date         : 2020-11-09 09:44:40
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-11-09 16:28:29
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/plugins/bluetooth/bluetooth-device.h
 */
#pragma once

#include <bluez_device_dbus_proxy.h>

namespace Kiran
{
class BluetoothDevice
{
public:
    BluetoothDevice(const std::string &object_path);
    virtual ~BluetoothDevice(){};

public:
    std::string get_object_path() { return this->object_path_; };
    std::string get_adapter() { return this->device_proxy_->Adapter_get().raw(); };

private:
    void init();

private:
    Glib::RefPtr<bluez::Device1Proxy> device_proxy_;

    std::string object_path_;

    std::string adapter_;
};

using BluetoothDeviceVec = std::vector<std::shared_ptr<BluetoothDevice>>;
}  // namespace Kiran