/*
 * @Author       : tangjie02
 * @Date         : 2020-06-18 17:34:09
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-09-04 17:54:51
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/plugins/display/display-plugin.cpp
 */

#include "plugins/display/display-plugin.h"

#include <cstdio>

#include "lib/base/log.h"
#include "plugins/display/display-manager.h"
#include "plugins/display/xrandr-manager.h"

PLUGIN_EXPORT_FUNC_DEF(DisplayPlugin);

namespace Kiran
{
DisplayPlugin::DisplayPlugin()
{
}

DisplayPlugin::~DisplayPlugin()
{
}

void DisplayPlugin::activate()
{
    SETTINGS_PROFILE("active display plugin.");
    XrandrManager::global_init();
    DisplayManager::global_init(XrandrManager::get_instance());
}

void DisplayPlugin::deactivate()
{
    SETTINGS_PROFILE("deactive display plugin.");
    DisplayManager::global_deinit();
    XrandrManager::global_deinit();
}
}  // namespace Kiran