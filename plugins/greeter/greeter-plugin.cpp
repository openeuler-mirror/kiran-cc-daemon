/**
 * @file          /kiran-cc-daemon/plugins/greeter/greeter-plugin.cpp
 * @brief description
 * @author yangxiaoqing <yangxiaoqing@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved.
*/
#include "plugins/greeter/greeter-plugin.h"
#include <cstdio>

#include "lib/base/log.h"
#include "plugins/greeter/greeter-dbus.h"

PLUGIN_EXPORT_FUNC_DEF(GreeterPlugin);

namespace Kiran
{
GreeterPlugin::GreeterPlugin()
{
}

GreeterPlugin::~GreeterPlugin()
{
}

void GreeterPlugin::activate()
{
    SETTINGS_PROFILE("active greeter settings plugin.");

    // GreeterSettingsWrapper::global_init();
    GreeterDBus::global_init();
}

void GreeterPlugin::deactivate()
{
    SETTINGS_PROFILE("deactive greeter settings plugin.");

    GreeterDBus::global_deinit();
    //GreeterSettingsWrapper::global_deinit();
}

}  // namespace Kiran
