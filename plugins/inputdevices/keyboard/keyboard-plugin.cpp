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

#include "plugins/inputdevices/keyboard/keyboard-plugin.h"

#include <cstdio>

#include <gtk3-log-i.h>
#include "plugins/inputdevices/keyboard/keyboard-manager.h"

PLUGIN_EXPORT_FUNC_DEF(KeyboardPlugin);

#define MATE_KEYBOARD_SCHEMA_ID "org.mate.SettingsDaemon.plugins.keyboard"
#define MATE_KEYBOARD_SCHEMA_KEY_ACTIVE "active"

namespace Kiran
{
KeyboardPlugin::KeyboardPlugin()
{
}

KeyboardPlugin::~KeyboardPlugin()
{
}

void KeyboardPlugin::activate()
{
    KLOG_PROFILE("active keyboard plugin.");

    // kiran和mate的插件最好不要同时运行，如果开启了kiran的插件，则将mate的插件停用
    auto schemas = Gio::Settings::list_schemas();
    if (std::find(schemas.begin(), schemas.end(), MATE_KEYBOARD_SCHEMA_ID) != schemas.end())
    {
        auto mate_keyboard = Gio::Settings::create(MATE_KEYBOARD_SCHEMA_ID);
        if (mate_keyboard->get_boolean(MATE_KEYBOARD_SCHEMA_KEY_ACTIVE))
        {
            mate_keyboard->set_boolean(MATE_KEYBOARD_SCHEMA_KEY_ACTIVE, false);
        }
    }

    KeyboardManager::global_init();
}

void KeyboardPlugin::deactivate()
{
    KLOG_PROFILE("deactive keyboard plugin.");

    auto schemas = Gio::Settings::list_schemas();
    if (std::find(schemas.begin(), schemas.end(), MATE_KEYBOARD_SCHEMA_ID) != schemas.end())
    {
        auto mate_keyboard = Gio::Settings::create(MATE_KEYBOARD_SCHEMA_ID);
        if (!mate_keyboard->get_boolean(MATE_KEYBOARD_SCHEMA_KEY_ACTIVE))
        {
            mate_keyboard->set_boolean(MATE_KEYBOARD_SCHEMA_KEY_ACTIVE, true);
        }
    }

    KeyboardManager::global_deinit();
}
}  // namespace Kiran