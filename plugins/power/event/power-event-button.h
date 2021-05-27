/**
 * @file          /kiran-cc-daemon/plugins/power/event/power-event-button.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
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