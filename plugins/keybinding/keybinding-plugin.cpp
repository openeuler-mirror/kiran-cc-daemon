/*
 * @Author       : tangjie02
 * @Date         : 2020-06-18 17:34:09
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-08-24 17:11:07
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/plugins/keybinding/keybinding-plugin.cpp
 */

#include "plugins/keybinding/keybinding-plugin.h"

#include <cstdio>

#include "lib/log.h"
#include "plugins/keybinding/keybinding-manager.h"

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
    SETTINGS_PROFILE("active keybinding plugin.");
    KeybindingManager::global_init();
}

void KeybindingPlugin::deactivate()
{
    SETTINGS_PROFILE("deactive keybinding plugin.");
    KeybindingManager::global_deinit();
}
}  // namespace Kiran