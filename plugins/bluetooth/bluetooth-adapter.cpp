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