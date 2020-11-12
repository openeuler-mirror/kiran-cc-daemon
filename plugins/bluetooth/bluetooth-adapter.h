/*
 * @Author       : tangjie02
 * @Date         : 2020-11-09 09:44:40
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-11-09 16:28:39
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/plugins/bluetooth/bluetooth-adapter.h
 */
#pragma once

#include <bluez_adapter_dbus_proxy.h>

#include "plugins/bluetooth/bluetooth-device.h"

namespace Kiran
{
class BluetoothAdapter
{
public:
    BluetoothAdapter(const std::string &object_path);
    virtual ~BluetoothAdapter(){};

    // 获取当前适配器的object path
    std::string get_object_path() { return this->object_path_; };
    // 添加设备
    bool add_device(std::shared_ptr<BluetoothDevice> device);
    // 删除设备
    bool remove_device(const std::string &object_path);
    // 通过设备的object_path查找设备对象
    std::shared_ptr<BluetoothDevice> find_device(const std::string &object_path);
    // 获取所有设备
    BluetoothDeviceVec get_devices();

private:
    // 当前适配器的object path
    std::string object_path_;
    std::map<std::string, std::shared_ptr<BluetoothDevice>> devices_;
};
}  // namespace Kiran