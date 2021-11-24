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

#include "plugins/power/power-plugin.h"

#include <gtk3-log-i.h>
#include "plugins/power/backlight/power-backlight.h"
#include "plugins/power/event/power-event-control.h"
#include "plugins/power/idle/power-idle-control.h"
#include "plugins/power/notification/power-notification-manager.h"
#include "plugins/power/power-manager.h"
#include "plugins/power/save/power-save.h"
#include "plugins/power/tray/power-tray.h"
#include "plugins/power/wrapper/power-wrapper-manager.h"

PLUGIN_EXPORT_FUNC_DEF(PowerPlugin);

namespace Kiran
{
PowerPlugin::PowerPlugin()
{
}

PowerPlugin::~PowerPlugin()
{
}

void PowerPlugin::activate()
{
    KLOG_PROFILE("active power plugin.");

    PowerWrapperManager::global_init();
    PowerBacklight::global_init();
    PowerManager::global_init(PowerWrapperManager::get_instance(), PowerBacklight::get_instance());
    PowerSave::global_init(PowerWrapperManager::get_instance());
    PowerIdleControl::global_init(PowerWrapperManager::get_instance(), PowerBacklight::get_instance());
    PowerEventControl::global_init(PowerWrapperManager::get_instance(), PowerBacklight::get_instance());
    PowerNotificationManager::global_init(PowerWrapperManager::get_instance());
}

void PowerPlugin::deactivate()
{
    KLOG_PROFILE("deactive power plugin.");

    PowerNotificationManager::global_deinit();
    PowerEventControl::global_deinit();
    PowerIdleControl::global_deinit();
    PowerSave::global_deinit();
    PowerManager::global_deinit();
    PowerBacklight::global_deinit();
    PowerWrapperManager::global_deinit();
}
}  // namespace Kiran