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