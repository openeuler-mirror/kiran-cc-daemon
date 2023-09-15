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

#include "plugins/inputdevices/mouse/mouse-plugin.h"

#include <cstdio>

#include <gtk3-log-i.h>
#include "plugins/inputdevices/mouse/mouse-manager.h"

PLUGIN_EXPORT_FUNC_DEF(MousePlugin);

namespace Kiran
{
MousePlugin::MousePlugin()
{
}

MousePlugin::~MousePlugin()
{
}

void MousePlugin::activate()
{
    KLOG_PROFILE("active mouse plugin.");

    MouseManager::global_init();
}

void MousePlugin::deactivate()
{
    KLOG_PROFILE("deactive mouse plugin.");

    MouseManager::global_deinit();
}
}  // namespace Kiran
