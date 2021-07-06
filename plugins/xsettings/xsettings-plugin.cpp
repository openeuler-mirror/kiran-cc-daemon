/**
 * @Copyright (C) 2020 ~ 2021 KylinSec Co., Ltd. 
 *
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; If not, see <http: //www.gnu.org/licenses/>. 
 */

#include "plugins/xsettings/xsettings-plugin.h"

#include <cstdio>

#include <gtk3-log-i.h>
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
    KLOG_PROFILE("active xsettings plugin.");

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
    KLOG_PROFILE("deactive xsettings plugin.");
    XSettingsManager::global_deinit();
}
}  // namespace Kiran