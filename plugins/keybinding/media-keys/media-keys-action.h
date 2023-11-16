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

#include "lib/base/base.h"
#include "lib/osdwindow/osd-window.h"
#include "plugins/keybinding/media-keys/media-keys-audio.h"

#include <X11/Xlib.h>

namespace Kiran
{
class MediaKeysAction
{
public:
    MediaKeysAction();
    ~MediaKeysAction();

    void init();

    bool do_action(XEvent* xev, std::string name);

private:
    void init_touchpad();

    void do_touchpad();

    void do_touchpad_osd(bool state);

    void do_shutdown();

    void do_logout();

    void do_eject();

    void do_screensaver();

    void do_home();

    void do_search();

    void do_control_center();

    void do_calculator();

    void do_media();

    void do_appinfo_for_uri(const Glib::ustring& uri);

    void do_screen_toggle(const Glib::ustring& key);

    void do_show_desktop();

    void do_panel_kiran_menu(XEvent* xev);

    void do_eject_action_cb(Glib::RefPtr<Gio::AsyncResult>& result, Glib::RefPtr<Gio::Drive> drive);

private:
    bool has_touchpad_;

    std::shared_ptr<MediaKeysAudio> audio_;

    Glib::RefPtr<Gio::DBus::Proxy> touchpad_proxy_;
};

}  // namespace Kiran
