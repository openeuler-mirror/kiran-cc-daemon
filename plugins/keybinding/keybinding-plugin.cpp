/*
 * @Author       : tangjie02
 * @Date         : 2020-06-18 17:34:09
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-09-02 15:25:53
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/plugins/keybinding/keybinding-plugin.cpp
 */

#include "plugins/keybinding/keybinding-plugin.h"

#include "lib/base/log.h"
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
    SETTINGS_PROFILE("active keybinding plugin.");
    CustomShortCutManager::global_init();
    SystemShortCutManager::global_init();
    KeybindingManager::global_init(SystemShortCutManager::get_instance());
}

void KeybindingPlugin::deactivate()
{
    SETTINGS_PROFILE("deactive keybinding plugin.");
    KeybindingManager::global_deinit();
    SystemShortCutManager::global_init();
    CustomShortCutManager::global_deinit();
}
}  // namespace Kiran