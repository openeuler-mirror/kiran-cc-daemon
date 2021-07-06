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
        KLOG_WARNING("The device %s already exist.", device->get_object_path().c_str());
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