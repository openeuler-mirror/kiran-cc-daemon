/**
 * @Copyright (C) 2020 ~ 2021 KylinSec Co., Ltd. 
 *
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; If not, see <http: //www.gnu.org/licenses/>. 
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