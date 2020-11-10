/*
 * @Author       : tangjie02
 * @Date         : 2020-06-18 17:34:09
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-09-02 15:29:39
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/plugins/bluetooth/bluetooth-plugin.cpp
 */

#include "plugins/bluetooth/bluetooth-plugin.h"

#include "lib/base/log.h"
#include "plugins/bluetooth/bluetooth-manager.h"

PLUGIN_EXPORT_FUNC_DEF(BluetoothPlugin);

namespace Kiran
{
BluetoothPlugin::BluetoothPlugin()
{
}

BluetoothPlugin::~BluetoothPlugin()
{
}

void BluetoothPlugin::activate()
{
    SETTINGS_PROFILE("active bluetooth plugin.");
    BluetoothManager::global_init();
}

void BluetoothPlugin::deactivate()
{
    SETTINGS_PROFILE("deactive bluetooth plugin.");
    BluetoothManager::global_deinit();
}
}  // namespace Kiran