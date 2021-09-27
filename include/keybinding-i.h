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

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#define KEYBINDING_DBUS_NAME "com.kylinsec.Kiran.SessionDaemon.Keybinding"
#define KEYBINDING_OBJECT_PATH "/com/kylinsec/Kiran/SessionDaemon/Keybinding"
#define KEYBINDING_DBUS_INTERFACE_NAME "com.kylinsec.Kiran.SessionDaemon.Keybinding"

// JK: JSON KEY
#define KEYBINDING_SHORTCUT_JK_UID "uid"
#define KEYBINDING_SHORTCUT_JK_NAME "name"
#define KEYBINDING_SHORTCUT_JK_ACTION "action"
#define KEYBINDING_SHORTCUT_JK_KEY_COMBINATION "key_combination"

#ifdef __cplusplus
}
#endif