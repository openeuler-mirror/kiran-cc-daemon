/*
 * @Author       : tangjie02
 * @Date         : 2020-06-18 17:34:09
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-08-06 15:59:24
 * @Description  : 
 * @FilePath     : /kiran-system-daemon/plugins/inputdevices/mouse/mouse-plugin.cpp
 */

#include "plugins/inputdevices/mouse/mouse-plugin.h"

#include <cstdio>

#include "lib/log.h"
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
    SETTINGS_PROFILE("active mouse plugin.");
    MouseManager::global_init();
}

void MousePlugin::deactivate()
{
    SETTINGS_PROFILE("deactive mouse plugin.");
    MouseManager::global_deinit();
}
}  // namespace Kiran