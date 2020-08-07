/*
 * @Author       : tangjie02
 * @Date         : 2020-06-18 17:34:09
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-08-06 17:20:52
 * @Description  : 
 * @FilePath     : /kiran-system-daemon/plugins/inputdevices/touchpad/touchpad-plugin.cpp
 */

#include "plugins/inputdevices/touchpad/touchpad-plugin.h"

#include <cstdio>

#include "lib/log.h"
#include "plugins/inputdevices/touchpad/touchpad-manager.h"

PLUGIN_EXPORT_FUNC_DEF(TouchPadPlugin);

namespace Kiran
{
TouchPadPlugin::TouchPadPlugin()
{
}

TouchPadPlugin::~TouchPadPlugin()
{
}

void TouchPadPlugin::activate()
{
    SETTINGS_PROFILE("active touchpad plugin.");
    TouchPadManager::global_init();
}

void TouchPadPlugin::deactivate()
{
    SETTINGS_PROFILE("deactive touchpad plugin.");
    TouchPadManager::global_deinit();
}
}  // namespace Kiran