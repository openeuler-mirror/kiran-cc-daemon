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
 * Author:     meizhigang <meizhigang@kylinos.com.cn>
 */

#include "plugins/network/network-plugin.h"
#include <gtk3-log-i.h>
#include "plugins/network/network-proxy-manager.h"

PLUGIN_EXPORT_FUNC_DEF(NetworkPlugin);

namespace Kiran
{
NetworkPlugin::NetworkPlugin()
{
}

NetworkPlugin::~NetworkPlugin()
{
}

void NetworkPlugin::activate()
{
    KLOG_DEBUG_NETWORK("Active network plugin.");

    NetworkProxyManager::global_init();
}

void NetworkPlugin::deactivate()
{
    KLOG_DEBUG_NETWORK("Deactive network plugin.");
    NetworkProxyManager::global_deinit();
}
}  // namespace Kiran