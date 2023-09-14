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
#include "plugins/inputdevices/keyboard/modifier-lock-manager.h"
//
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
    KLOG_PROFILE("active keyboard plugin.");

    KeyboardManager::global_init();
    ModifierLockManager::global_init(KeyboardManager::get_instance());
}

void KeyboardPlugin::deactivate()
{
    KLOG_PROFILE("deactive keyboard plugin.");

    ModifierLockManager::global_deinit();
    KeyboardManager::global_deinit();
}
}  // namespace Kiran
