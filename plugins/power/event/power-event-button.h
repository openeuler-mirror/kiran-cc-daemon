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
#include <gdk/gdkx.h>

#include "lib/base/base.h"
#include "plugins/power/wrapper/power-wrapper-manager.h"
#include "power-i.h"

namespace Kiran
{
class PowerEventButton
{
public:
    PowerEventButton();
    virtual ~PowerEventButton();

    void init();

    // 按键被按下的信号
    sigc::signal<void, PowerEvent> signal_button_changed() { return this->button_changed_; };

private:
    bool register_button(uint32_t keysym, PowerEvent type);

    // 发送按键信号，如果跟上一次发送的按键信号相同且时间间隔较短，则忽略该次按键信号的发送
    void emit_button_signal(PowerEvent type);

    void on_lid_is_closed_change(bool lid_is_closed);
    static GdkFilterReturn window_event(GdkXEvent *gdk_event, GdkEvent *event, gpointer data);

private:
    GdkDisplay *display_;
    Display *xdisplay_;

    GdkWindow *root_window_;
    Window xroot_window_;

    std::shared_ptr<PowerUPower> upower_client_;

    std::map<std::string, PowerEvent> buttons_;
    Glib::Timer button_signal_timer_;

    sigc::signal<void, PowerEvent> button_changed_;
};
}  // namespace Kiran