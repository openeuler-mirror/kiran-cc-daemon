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