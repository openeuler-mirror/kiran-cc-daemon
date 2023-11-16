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
#include "lib/base/base.h"
#include "plugins/keybinding/keybinding-manager.h"
#include "plugins/keybinding/media-keys/media-keys-action.h"

namespace Kiran
{
struct MediaKeysShortCut
{
    std::string uid;
    // 快捷键
    std::string key_combination;
    // 快捷键在gsettings中的key
    std::string settings_key;
};

class MediaKeysManager
{
public:
    MediaKeysManager(KeybindingManager *keybinding_manager);
    virtual ~MediaKeysManager();

    static MediaKeysManager *get_instance() { return instance_; }

    static void global_init(KeybindingManager *keybinding_manager);

    static void global_deinit() { delete instance_; }

    static GdkFilterReturn window_event(GdkXEvent *gdk_event, GdkEvent *event, gpointer data);

private:
    void init();

    void init_modifiers();

    void init_grab_keys();

    bool is_valid_key_event(XEvent *xev);

    void system_shortcut_added(std::shared_ptr<SystemShortCut> system_shortcut);

    void system_shortcut_deleted(std::shared_ptr<SystemShortCut> system_shortcut);

    void system_shortcut_changed(std::shared_ptr<SystemShortCut> system_shortcut);

private:
    static MediaKeysManager *instance_;

    Glib::RefPtr<Gdk::Window> root_window_;

    std::shared_ptr<SystemShortCuts> system_shortcuts_;
    std::map<std::string, std::shared_ptr<MediaKeysShortCut>> shortcuts_;

    std::shared_ptr<MediaKeysAction> actions_;

    uint32_t ignored_mods_;
    uint32_t used_mods_;

    bool super_valid_flag_;
};

}  // namespace Kiran