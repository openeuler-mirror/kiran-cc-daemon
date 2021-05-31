/**
 * @file          /kiran-cc-daemon/plugins/inputdevices/mouse/mouse-plugin.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#include "plugins/inputdevices/mouse/mouse-plugin.h"

#include <cstdio>

#include "lib/base/log.h"
#include "plugins/inputdevices/mouse/mouse-manager.h"

PLUGIN_EXPORT_FUNC_DEF(MousePlugin);

#define MATE_MOUSE_SCHEMA_ID "org.mate.SettingsDaemon.plugins.mouse"
#define MATE_MOUSE_SCHEMA_KEY_ACTIVE "active"

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
    MouseManager::global_init();
}

void MousePlugin::deactivate()
{
    SETTINGS_PROFILE("deactive mouse plugin.");
    MouseManager::global_deinit();
}
}  // namespace Kiran