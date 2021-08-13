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