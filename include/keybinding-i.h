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

/* The example of the shotcut list in json.
{"system":[
           {"key_combination":"key_comb_1","kind":"kind_1","name":"name_1","uid":"uid_1"},
           {"key_combination":"key_comb_2","kind":"kind_2","name":"name_2","uid":"uid_2"},
             ...
            ]
}
{"custom":[
           {"key_combination":"key_comb_1","action":"action_1","name":"name_1","uid":"uid_1"},
           {"key_combination":"key_comb_2","action":"action_2","name":"name_2","uid":"uid_2"},
             ...
          ]
}
*/

// JK: JSON KEY
#define KEYBINDING_SHORTCUT_JK_UID "uid"
#define KEYBINDING_SHORTCUT_JK_KIND "kind"
#define KEYBINDING_SHORTCUT_JK_NAME "name"
#define KEYBINDING_SHORTCUT_JK_ACTION "action"
#define KEYBINDING_SHORTCUT_JK_KEY_COMBINATION "key_combination"

#define KEYBINDING_SHORTCUT_JK_CUSTOM "custom"
#define KEYBINDING_SHORTCUT_JK_SYSTEM "system"

#ifdef __cplusplus
}
#endif