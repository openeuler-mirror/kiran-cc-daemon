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