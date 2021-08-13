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

#include "plugins/bluetooth/bluetooth-device.h"

#include "lib/base/base.h"
#include "plugins/bluetooth/bluez.h"

namespace Kiran
{
BluetoothDevice::BluetoothDevice(const std::string &object_path) : object_path_(object_path)
{
    this->init();
}

void BluetoothDevice::init()
{
    this->device_proxy_ = bluez::Device1Proxy::createForBus_sync(Gio::DBus::BUS_TYPE_SYSTEM,
                                                                 Gio::DBus::PROXY_FLAGS_NONE,
                                                                 BLUEZ_DBUS_NAME,
                                                                 this->object_path_);
}

}  // namespace Kiran