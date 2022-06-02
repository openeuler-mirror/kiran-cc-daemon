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
#include <gdkmm.h>
//
#include <X11/X.h>

namespace Kiran
{
// Extended Window Manager Hints
class EWMH
{
public:
    EWMH();
    virtual ~EWMH();

    static EWMH* get_instance() { return instance_; };

    static void global_init();

    static void global_deinit();

    // 获得窗口管理器keybingding名字列表
    std::vector<std::string> get_wm_keybindings();

    // 获取窗口管理器的窗口属性
    std::string get_wm_property(Atom atom);

    // 获取窗口管理器名字
    std::string get_wm_name();

    sigc::signal<void> signal_wm_window_change() { return this->wm_window_changed_; }

private:
    void init();
    // 更新窗口管理器在根窗口中设置的_NET_SUPPORTING_WM_CHECK属性，该属性值记录了窗口管理器的一个子窗口
    void update_wm_window();

    static GdkFilterReturn window_event(GdkXEvent* gdk_event, GdkEvent* event, gpointer data);

private:
    static EWMH* instance_;

    Window wm_window_;

    sigc::signal<void> wm_window_changed_;
};
}  // namespace Kiran