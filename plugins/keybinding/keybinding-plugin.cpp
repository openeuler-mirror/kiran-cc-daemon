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

#include "plugins/keybinding/keybinding-plugin.h"

#include <gtk3-log-i.h>
#include "plugins/keybinding/custom-shortcut.h"
#include "plugins/keybinding/keybinding-manager.h"
#include "plugins/keybinding/system-shortcut.h"

PLUGIN_EXPORT_FUNC_DEF(KeybindingPlugin);

#define MATE_KEYBINDINGS_SCHEMA_ID "org.mate.SettingsDaemon.plugins.keybindings"
#define MATE_KEYBINDINGS_SCHEMA_KEY_ACTIVE "active"

namespace Kiran
{
KeybindingPlugin::KeybindingPlugin()
{
}

KeybindingPlugin::~KeybindingPlugin()
{
}

void KeybindingPlugin::activate()
{
    KLOG_PROFILE("active keybinding plugin.");

    // kiran和mate的相同插件不要同时运行，如果开启了kiran的插件，则将mate的插件停用
    auto schemas = Gio::Settings::list_schemas();
    if (std::find(schemas.begin(), schemas.end(), MATE_KEYBINDINGS_SCHEMA_ID) != schemas.end())
    {
        auto mate_keybinding = Gio::Settings::create(MATE_KEYBINDINGS_SCHEMA_ID);
        if (mate_keybinding->get_boolean(MATE_KEYBINDINGS_SCHEMA_KEY_ACTIVE))
        {
            mate_keybinding->set_boolean(MATE_KEYBINDINGS_SCHEMA_KEY_ACTIVE, false);
        }
    }

    KeybindingManager::global_init();
}

void KeybindingPlugin::deactivate()
{
    KLOG_PROFILE("deactive keybinding plugin.");

    auto schemas = Gio::Settings::list_schemas();
    if (std::find(schemas.begin(), schemas.end(), MATE_KEYBINDINGS_SCHEMA_ID) != schemas.end())
    {
        auto mate_keybinding = Gio::Settings::create(MATE_KEYBINDINGS_SCHEMA_ID);
        if (!mate_keybinding->get_boolean(MATE_KEYBINDINGS_SCHEMA_KEY_ACTIVE))
        {
            mate_keybinding->set_boolean(MATE_KEYBINDINGS_SCHEMA_KEY_ACTIVE, true);
        }
    }

    KeybindingManager::global_deinit();
}
}  // namespace Kiran