/*
 * @Author       : tangjie02
 * @Date         : 2020-11-09 09:44:45
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-11-09 16:12:58
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/plugins/bluetooth/bluetooth-adapter.cpp
 */
#include "plugins/bluetooth/bluetooth-adapter.h"

#include "lib/base/base.h"

namespace Kiran
{
BluetoothAdapter::BluetoothAdapter(const std::string &object_path) : object_path_(object_path)
{
}

bool BluetoothAdapter::add_device(std::shared_ptr<BluetoothDevice> device)
{
    auto iter = this->devices_.emplace(device->get_object_path(), device);
    if (!iter.second)
    {
        LOG_WARNING("The device %s already exist.", device->get_object_path());
    }
    return iter.second;
}

bool BluetoothAdapter::remove_device(const std::string &object_path)
{
    auto iter = this->devices_.find(object_path);
    if (iter == this->devices_.end())
    {
        return false;
    }
    this->devices_.erase(iter);
    return true;
}

std::shared_ptr<BluetoothDevice> BluetoothAdapter::find_device(const std::string &object_path)
{
    auto iter = this->devices_.find(object_path);
    if (iter == this->devices_.end())
    {
        return nullptr;
    }

    return iter->second;
}

BluetoothDeviceVec BluetoothAdapter::get_devices()
{
    BluetoothDeviceVec devices;
    for (auto &iter : this->devices_)
    {
        devices.push_back(iter.second);
    }
    return devices;
}
}  // namespace Kiran