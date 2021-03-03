/**
 * @file greeter-settings-plugin.cpp
 * @brief description
 * @author yangxiaoqing <yangxiaoqing@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved.
*/
#include "greeter-settings-plugin.h"
#include <cstdio>

#include "lib/base/log.h"
#include "plugins/greeter-settings/greeter-settings-dbus.h"

PLUGIN_EXPORT_FUNC_DEF(GreeterSettingsPlugin);

namespace Kiran
{

GreeterSettingsPlugin::GreeterSettingsPlugin()
{

}

GreeterSettingsPlugin::~GreeterSettingsPlugin()
{

}

void GreeterSettingsPlugin::activate()
{
    SETTINGS_PROFILE("active greeter settings plugin.");

   // GreeterSettingsWrapper::global_init();
    GreeterSettingsDbus::global_init();
}

void GreeterSettingsPlugin::deactivate()
{
    SETTINGS_PROFILE("deactive greeter settings plugin.");

    GreeterSettingsDbus::global_deinit();
    //GreeterSettingsWrapper::global_deinit();
}

}
