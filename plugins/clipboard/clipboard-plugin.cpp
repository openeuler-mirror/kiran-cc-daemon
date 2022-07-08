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
 * Author:     meizhigang <meizhigang@kylinos.com.cn>
 */

#include "plugins/clipboard/clipboard-plugin.h"
#include <gtk3-log-i.h>
#include "plugins/clipboard/clipboard-manager.h"

PLUGIN_EXPORT_FUNC_DEF(ClipboardPlugin);

#define MATE_CLIPBOARD_SCHEMA_ID "org.mate.SettingsDaemon.plugins.clipboard"
#define MATE_CLIPBOARD_SCHEMA_KEY_ACTIVE "active"

namespace Kiran
{
ClipboardPlugin::ClipboardPlugin()
{
}

ClipboardPlugin::~ClipboardPlugin()
{
}

void ClipboardPlugin::activate()
{
    KLOG_PROFILE("active clipboard plugin.");

    // kiran和mate的插件最好不要同时运行，如果开启了kiran的插件，则将mate的插件停用
    auto schemas = Gio::Settings::list_schemas();
    if (std::find(schemas.begin(), schemas.end(), MATE_CLIPBOARD_SCHEMA_ID) != schemas.end())
    {
        auto mate_clipboard = Gio::Settings::create(MATE_CLIPBOARD_SCHEMA_ID);
        if (mate_clipboard->get_boolean(MATE_CLIPBOARD_SCHEMA_KEY_ACTIVE))
        {
            mate_clipboard->set_boolean(MATE_CLIPBOARD_SCHEMA_KEY_ACTIVE, false);
        }
    }

    ClipboardManager::global_init();
}

void ClipboardPlugin::deactivate()
{
    KLOG_PROFILE("deactive clipboard plugin.");

    auto schemas = Gio::Settings::list_schemas();
    if (std::find(schemas.begin(), schemas.end(), MATE_CLIPBOARD_SCHEMA_ID) != schemas.end())
    {
        auto mate_clipboard = Gio::Settings::create(MATE_CLIPBOARD_SCHEMA_ID);
        if (!mate_clipboard->get_boolean(MATE_CLIPBOARD_SCHEMA_KEY_ACTIVE))
        {
            mate_clipboard->set_boolean(MATE_CLIPBOARD_SCHEMA_KEY_ACTIVE, true);
        }
    }

    ClipboardManager::global_deinit();
}
}  // namespace Kiran