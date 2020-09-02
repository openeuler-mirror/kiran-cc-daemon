/*
 * @Author       : tangjie02
 * @Date         : 2020-06-18 17:34:09
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-09-02 15:20:55
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/plugins/inputdevices/keyboard/keyboard-plugin.cpp
 */

#include "plugins/inputdevices/keyboard/keyboard-plugin.h"

#include <cstdio>

#include "lib/base/log.h"
#include "plugins/inputdevices/keyboard/keyboard-manager.h"

PLUGIN_EXPORT_FUNC_DEF(KeyboardPlugin);

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
    SETTINGS_PROFILE("active keyboard plugin.");
    KeyboardManager::global_init();
}

void KeyboardPlugin::deactivate()
{
    SETTINGS_PROFILE("deactive keyboard plugin.");
    KeyboardManager::global_deinit();
}
}  // namespace Kiran