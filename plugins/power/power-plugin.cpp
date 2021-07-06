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
    KLOG_PROFILE("active appearance plugin.");

    PowerWrapperManager::global_init();
    PowerBacklight::global_init();
    PowerManager::global_init(PowerWrapperManager::get_instance(), PowerBacklight::get_instance());
    PowerSave::global_init(PowerWrapperManager::get_instance());
    PowerIdleControl::global_init(PowerWrapperManager::get_instance(), PowerBacklight::get_instance());
    PowerEventControl::global_init(PowerWrapperManager::get_instance(), PowerBacklight::get_instance());
    PowerNotificationManager::global_init(PowerWrapperManager::get_instance());
    PowerTray::global_init(PowerWrapperManager::get_instance());
}

void PowerPlugin::deactivate()
{
    KLOG_PROFILE("deactive appearance plugin.");

    PowerTray::global_deinit();
    PowerNotificationManager::global_deinit();
    PowerEventControl::global_deinit();
    PowerIdleControl::global_deinit();
    PowerSave::global_deinit();
    PowerManager::global_deinit();
    PowerBacklight::global_deinit();
    PowerWrapperManager::global_deinit();
}
}  // namespace Kiran