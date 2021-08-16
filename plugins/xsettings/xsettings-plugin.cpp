/**
 * Copyright (c) 2020 ~ 2021 KylinSec Co., Ltd. 
 * kiran-cc-daemon is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2. 
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2 
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, 
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, 
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.  
 * See the Mulan PSL v2 for more details.  
 * 
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
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