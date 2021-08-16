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
 * Author:     yangxiaoqing <yangxiaoqing@kylinos.com.cn>
 */

#include "plugins/greeter/greeter-plugin.h"
#include <cstdio>

#include <gtk3-log-i.h>
#include "plugins/greeter/greeter-dbus.h"

PLUGIN_EXPORT_FUNC_DEF(GreeterPlugin);

namespace Kiran
{
GreeterPlugin::GreeterPlugin()
{
}

GreeterPlugin::~GreeterPlugin()
{
}

void GreeterPlugin::activate()
{
    KLOG_PROFILE("active greeter settings plugin.");

    // GreeterSettingsWrapper::global_init();
    GreeterDBus::global_init();
}

void GreeterPlugin::deactivate()
{
    KLOG_PROFILE("deactive greeter settings plugin.");

    GreeterDBus::global_deinit();
    //GreeterSettingsWrapper::global_deinit();
}

}  // namespace Kiran
