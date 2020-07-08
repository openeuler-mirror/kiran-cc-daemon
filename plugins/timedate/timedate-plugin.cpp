/*
 * @Author       : tangjie02
 * @Date         : 2020-07-06 09:59:51
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-07-06 10:06:29
 * @Description  : 
 * @FilePath     : /kiran-system-daemon/plugins/timedate/timedate-plugin.cpp
 */
#include "plugins/timedate/timedate-plugin.h"

#include <cstdio>

#include "lib/log.h"
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

}  // namespace Kiran