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

#include "plugins/appearance/appearance-plugin.h"

#include <gtk3-log-i.h>
#include "plugins/appearance/appearance-manager.h"

PLUGIN_EXPORT_FUNC_DEF(AppearancePlugin);

namespace Kiran
{
AppearancePlugin::AppearancePlugin()
{
}

AppearancePlugin::~AppearancePlugin()
{
}

void AppearancePlugin::activate()
{
    KLOG_DEBUG_APPEARANCE("Active appearance plugin.");

    AppearanceManager::global_init();
}

void AppearancePlugin::deactivate()
{
    KLOG_DEBUG_APPEARANCE("Deactive appearance plugin.");

    AppearanceManager::global_deinit();
}
}  // namespace Kiran
