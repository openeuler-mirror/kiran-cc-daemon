/*
 * @Author       : tangjie02
 * @Date         : 2020-11-09 10:12:41
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-11-09 10:43:16
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/plugins/bluetooth/bluetooth-device.cpp
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