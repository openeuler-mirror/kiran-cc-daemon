/*
 * @Author       : tangjie02
 * @Date         : 2020-12-01 10:14:02
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-12-01 10:17:03
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/plugins/appearance/appearance-plugin.cpp
 */

#include "plugins/appearance/appearance-plugin.h"

#include "lib/base/log.h"
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
    SETTINGS_PROFILE("active appearance plugin.");
    AppearanceManager::global_init();
}

void AppearancePlugin::deactivate()
{
    SETTINGS_PROFILE("deactive appearance plugin.");
    AppearanceManager::global_deinit();
}
}  // namespace Kiran