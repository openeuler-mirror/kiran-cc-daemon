/**
 * @file          /kiran-cc-daemon/plugins/display/display-plugin.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#include "plugins/display/display-plugin.h"

#include <cstdio>

#include "lib/base/log.h"
#include "plugins/display/display-manager.h"
#include "plugins/display/xrandr-manager.h"

PLUGIN_EXPORT_FUNC_DEF(DisplayPlugin);

#define MATE_XRANDR_SCHEMA_ID "org.mate.SettingsDaemon.plugins.xrandr"
#define MATE_XRANDR_SCHEMA_KEY_ACTIVE "active"

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
    // kiran和mate的插件最好不要同时运行，如果开启了kiran的插件，则将mate的插件停用
    auto schemas = Gio::Settings::list_schemas();
    if (std::find(schemas.begin(), schemas.end(), MATE_XRANDR_SCHEMA_ID) != schemas.end())
    {
        auto mate_xrandr = Gio::Settings::create(MATE_XRANDR_SCHEMA_ID);
        if (mate_xrandr->get_boolean(MATE_XRANDR_SCHEMA_KEY_ACTIVE))
        {
            mate_xrandr->set_boolean(MATE_XRANDR_SCHEMA_KEY_ACTIVE, false);
        }
    }

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