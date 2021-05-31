/**
 * @file          /kiran-cc-daemon/plugins/inputdevices/touchpad/touchpad-plugin.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#include "plugins/inputdevices/touchpad/touchpad-plugin.h"

#include <cstdio>

#include "lib/base/log.h"
#include "plugins/inputdevices/touchpad/touchpad-manager.h"

PLUGIN_EXPORT_FUNC_DEF(TouchPadPlugin);

#define MATE_MOUSE_SCHEMA_ID "org.mate.SettingsDaemon.plugins.mouse"
#define MATE_MOUSE_SCHEMA_KEY_ACTIVE "active"

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

    // kiran和mate的相同插件不要同时运行，如果开启了kiran的插件，则将mate的插件停用
    auto schemas = Gio::Settings::list_schemas();
    if (std::find(schemas.begin(), schemas.end(), MATE_MOUSE_SCHEMA_ID) != schemas.end())
    {
        auto mate_xrandr = Gio::Settings::create(MATE_MOUSE_SCHEMA_ID);
        if (mate_xrandr->get_boolean(MATE_MOUSE_SCHEMA_KEY_ACTIVE))
        {
            mate_xrandr->set_boolean(MATE_MOUSE_SCHEMA_KEY_ACTIVE, false);
        }
    }

    TouchPadManager::global_init();
}

void TouchPadPlugin::deactivate()
{
    SETTINGS_PROFILE("deactive touchpad plugin.");
    TouchPadManager::global_deinit();
}
}  // namespace Kiran