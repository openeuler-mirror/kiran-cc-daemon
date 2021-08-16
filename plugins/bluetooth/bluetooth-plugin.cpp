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

#include "plugins/bluetooth/bluetooth-plugin.h"

#include <gtk3-log-i.h>
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
    KLOG_PROFILE("active bluetooth plugin.");
    BluetoothManager::global_init();
}

void BluetoothPlugin::deactivate()
{
    KLOG_PROFILE("deactive bluetooth plugin.");
    BluetoothManager::global_deinit();
}
}  // namespace Kiran