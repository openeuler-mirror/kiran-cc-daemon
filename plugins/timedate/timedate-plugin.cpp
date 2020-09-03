/*
 * @Author       : tangjie02
 * @Date         : 2020-07-06 09:59:51
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-09-03 09:12:37
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/plugins/timedate/timedate-plugin.cpp
 */
#include "plugins/timedate/timedate-plugin.h"

#include <cstdio>

#include "lib/base/log.h"
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
    SETTINGS_PROFILE("active timedate plugin.");

    TimedateManager::global_init();
}

void TimedatePlugin::deactivate()
{
    SETTINGS_PROFILE("deactive timedate plugin.");

    TimedateManager::global_deinit();
}

}  // namespace Kiran