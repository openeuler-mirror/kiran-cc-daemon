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
 * Author:     meizhigang <meizhigang@kylinsec.com.cn>
 */

#pragma once

#include <gdkmm.h>
//
#include <X11/Xlib.h>
#include "plugins/inputdevices/keyboard/keyboard-manager.h"

namespace Kiran
{
class ModifierLockManager
{
public:
    ModifierLockManager(KeyboardManager *keyboard_manager);
    ~ModifierLockManager();

    static ModifierLockManager *get_instance() { return instance_; };

    static void global_init(KeyboardManager *keyboard_manager);

    static void global_deinit() { delete instance_; };

private:
    void init();

    int xkb_init();

    void set_lock_action(KeyCode keycode, unsigned int mods);

    static GdkFilterReturn window_event(GdkXEvent *gdk_event, GdkEvent *event, gpointer data);

private:
    static ModifierLockManager *instance_;

    int xkb_event_base_;

    unsigned int capslock_mask_;
    unsigned int numlock_mask_;
    KeyCode capslock_keycode_;
    KeyCode numlock_keycode_;

    KeyboardManager *keyboard_manager_;
};

}  // namespace Kiran