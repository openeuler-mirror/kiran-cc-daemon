/**
 * @file          /kiran-cc-daemon/plugins/power/power-plugin.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#include "plugins/power/power-plugin.h"

#include "lib/base/log.h"
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
    SETTINGS_PROFILE("active appearance plugin.");

    PowerWrapperManager::global_init();
    PowerManager::global_init(PowerWrapperManager::get_instance());
    PowerSave::global_init(PowerWrapperManager::get_instance());
    PowerBacklight::global_init();
    PowerIdleControl::global_init(PowerWrapperManager::get_instance(), PowerBacklight::get_instance());
    PowerEventControl::global_init(PowerWrapperManager::get_instance(), PowerBacklight::get_instance());
    PowerNotificationManager::global_init(PowerWrapperManager::get_instance());
    PowerTray::global_init(PowerWrapperManager::get_instance());
}

void PowerPlugin::deactivate()
{
    SETTINGS_PROFILE("deactive appearance plugin.");

    PowerTray::global_deinit();
    PowerNotificationManager::global_deinit();
    PowerEventControl::global_deinit();
    PowerIdleControl::global_deinit();
    PowerBacklight::global_deinit();
    PowerSave::global_deinit();
    PowerManager::global_deinit();
    PowerWrapperManager::global_deinit();
}
}  // namespace Kiran