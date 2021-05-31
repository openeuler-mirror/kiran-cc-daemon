/**
 * @file          /kiran-cc-daemon/plugins/xsettings/xsettings-plugin.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#include "plugins/xsettings/xsettings-plugin.h"

#include <cstdio>

#include "lib/base/log.h"
#include "plugins/xsettings/xsettings-manager.h"

PLUGIN_EXPORT_FUNC_DEF(XSettingsPlugin);

#define MATE_XSETTINGS_SCHEMA_ID "org.mate.SettingsDaemon.plugins.xsettings"
#define MATE_XSETTINGS_SCHEMA_KEY_ACTIVE "active"

namespace Kiran
{
XSettingsPlugin::XSettingsPlugin()
{
}

XSettingsPlugin::~XSettingsPlugin()
{
}

void XSettingsPlugin::activate()
{
    SETTINGS_PROFILE("active xsettings plugin.");

    // kiran和mate的xrandr插件只能运行一个，如果开启了kiran的插件，则将mate的插件停用
    auto schemas = Gio::Settings::list_schemas();
    if (std::find(schemas.begin(), schemas.end(), MATE_XSETTINGS_SCHEMA_ID) != schemas.end())
    {
        auto mate_xrandr = Gio::Settings::create(MATE_XSETTINGS_SCHEMA_ID);
        if (mate_xrandr->get_boolean(MATE_XSETTINGS_SCHEMA_KEY_ACTIVE))
        {
            mate_xrandr->set_boolean(MATE_XSETTINGS_SCHEMA_KEY_ACTIVE, false);
            // 停顿0.1秒让mate关闭插件
            usleep(100000);
        }
    }

    XSettingsManager::global_init();
}

void XSettingsPlugin::deactivate()
{
    SETTINGS_PROFILE("deactive xsettings plugin.");
    XSettingsManager::global_deinit();
}
}  // namespace Kiran