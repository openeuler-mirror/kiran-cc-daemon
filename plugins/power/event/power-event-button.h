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

    // 抑制systemd-login1对电源、休眠、挂起按键和合上盖子进行操作。
    int32_t login1_inhibit_fd_;

    std::shared_ptr<PowerUPower> upower_client_;

    std::map<std::string, PowerEvent> buttons_;
    Glib::Timer button_signal_timer_;

    sigc::signal<void, PowerEvent> button_changed_;
};
}  // namespace Kiran