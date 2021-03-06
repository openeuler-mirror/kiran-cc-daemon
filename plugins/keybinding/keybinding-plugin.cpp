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
    CustomShortCutManager::global_init();
    SystemShortCutManager::global_init();
    KeybindingManager::global_init(SystemShortCutManager::get_instance());
}

void KeybindingPlugin::deactivate()
{
    KLOG_PROFILE("deactive keybinding plugin.");
    KeybindingManager::global_deinit();
    SystemShortCutManager::global_init();
    CustomShortCutManager::global_deinit();
}
}  // namespace Kiran