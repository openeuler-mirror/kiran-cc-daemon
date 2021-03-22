/**
 * @file          /kiran-cc-daemon/plugins/systeminfo/systeminfo-plugin.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */
#include "plugins/systeminfo/systeminfo-plugin.h"

#include <cstdio>

#include "lib/base/log.h"
#include "plugins/systeminfo/systeminfo-manager.h"

PLUGIN_EXPORT_FUNC_DEF(SystemInfoPlugin);

namespace Kiran
{
SystemInfoPlugin::SystemInfoPlugin()
{
}

SystemInfoPlugin::~SystemInfoPlugin()
{
}

void SystemInfoPlugin::activate()
{
    SETTINGS_PROFILE("active systeminfo plugin.");

    SystemInfoManager::global_init();
}

void SystemInfoPlugin::deactivate()
{
    SETTINGS_PROFILE("deactive systeminfo plugin.");

    SystemInfoManager::global_deinit();
}

}  // namespace Kiran