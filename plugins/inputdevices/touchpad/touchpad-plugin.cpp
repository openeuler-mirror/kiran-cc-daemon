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

#include "plugins/inputdevices/touchpad/touchpad-plugin.h"

#include <cstdio>

#include <gtk3-log-i.h>
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
    KLOG_PROFILE("active touchpad plugin.");

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
    KLOG_PROFILE("deactive touchpad plugin.");
    TouchPadManager::global_deinit();
}
}  // namespace Kiran