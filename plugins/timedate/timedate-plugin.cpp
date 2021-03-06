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

#include "plugins/timedate/timedate-plugin.h"

#include <cstdio>

#include <gtk3-log-i.h>
#include "lib/dbus/auth-manager.h"
#include "plugins/timedate/timedate-manager.h"

PLUGIN_EXPORT_FUNC_DEF(TimedatePlugin);

namespace Kiran
{
TimedatePlugin::TimedatePlugin()
{
}

TimedatePlugin::~TimedatePlugin()
{
}

void TimedatePlugin::activate()
{
    KLOG_PROFILE("active timedate plugin.");

    TimedateManager::global_init();
}

void TimedatePlugin::deactivate()
{
    KLOG_PROFILE("deactive timedate plugin.");

    TimedateManager::global_deinit();
}

}  // namespace Kiran