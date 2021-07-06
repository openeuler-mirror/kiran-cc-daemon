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