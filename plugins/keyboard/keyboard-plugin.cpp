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

#include "keyboard-plugin.h"
#include "iso-translation.h"
#include "keyboard-manager.h"
#include "modifier-lock-manager.h"

namespace Kiran
{
void KeyboardPlugin::activate()
{
    ISOTranslation::globalInit();
    KeyboardManager::globalInit();
    ModifierLockManager::globalInit(KeyboardManager::getInstance());
}

void KeyboardPlugin::deactivate()
{
    KeyboardManager::globalDeinit();
    ISOTranslation::globalDeinit();
}
}  // namespace Kiran
