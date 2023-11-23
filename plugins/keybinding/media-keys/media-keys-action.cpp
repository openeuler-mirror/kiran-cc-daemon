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
#include "plugins/keybinding/media-keys/media-keys-action.h"
#include "media-keys-i.h"
#include "plugins/inputdevices/common/xinput-helper.h"

#include "audio-i.h"
#include "network-i.h"
#include "touchpad-i.h"

#define WNCK_I_KNOW_THIS_IS_UNSTABLE
#include <libwnck/libwnck.h>

namespace Kiran
{
#define APPINFO_URI_EMAIL "mailto"
#define APPINFO_URI_WWW "http"
#define APPINFO_URI_HELP "help"
#define APPINFO_CONTEXT_MEDIA "audio/x-vorbis+ogg"

#define DESKTOP_APP_SCHEMA_ID "org.gnome.desktop.a11y.applications"
#define SCREEN_MAGNIFIER_ENABLED_KEY "screen-magnifier-enabled"
#define SCREEN_READER_ENABLED_KEY "screen-reader-enabled"
#define SCREEN_KEYBOARD_ENABLED_KEY "screen-keyboard-enabled"

#define TOUCHPAD_SCHEMA_ID "com.kylinsec.kiran.touchpad"
#define TOUCHPAD_SCHEMA_TOUCHPAD_ENABLED "touchpad-enabled"

#define IMAGE_TOUCHPAD_ENABLED "osd-touchpad-enabled"
#define IMAGE_TOUCHPAD_DISABLED "osd-touchpad-disabled"
#define IMAGE_MEDIA_EJECT "osd-media-eject"

MediaKeysAction::MediaKeysAction() : has_touchpad_(false)
{
    this->audio_ = std::make_shared<MediaKeysAudio>();
}

MediaKeysAction::~MediaKeysAction()
{
}

void MediaKeysAction::init()
{
    this->init_touchpad();
    this->audio_->init();
}

void MediaKeysAction::init_touchpad()
{
    if (!XInputHelper::supports_xinput_devices())
    {
        KLOG_WARNING_KEYBINDING("XInput is not supported, not applying any settings.");
    }

    XInputHelper::foreach_device([this](std::shared_ptr<DeviceHelper> device_helper)
                                 {
                                     if (device_helper->is_touchpad())
                                     {
                                         this->has_touchpad_ = true;
                                     }
                                 });
}

bool MediaKeysAction::do_action(XEvent *xev, std::string name)
{
    switch (shash(name.c_str()))
    {
    case CONNECT(MEDIAKEYS_SCHEMA_TOUCHPAD, _hash):
        this->do_touchpad();
        break;
    case CONNECT(MEDIAKEYS_SCHEMA_TOUCHPAD_ON, _hash):
        this->do_touchpad_osd(true);
        break;
    case CONNECT(MEDIAKEYS_SCHEMA_TOUCHPAD_OFF, _hash):
        this->do_touchpad_osd(false);
        break;
    case CONNECT(MEDIAKEYS_SCHEMA_MIC_MUTE, _hash):
        this->audio_->do_sound_action(VOLUME_MIC_MUTE);
        break;
    case CONNECT(MEDIAKEYS_SCHEMA_VOLUME_MUTE, _hash):
    case CONNECT(MEDIAKEYS_SCHEMA_VOLUME_MUTE_QUIET, _hash):
        this->audio_->do_sound_action(VOLUME_MUTE);
        break;
    case CONNECT(MEDIAKEYS_SCHEMA_VOLUME_DOWN, _hash):
    case CONNECT(MEDIAKEYS_SCHEMA_VOLUME_DOWN_QUIET, _hash):
        this->audio_->do_sound_action(VOLUME_DOWN);
        break;
    case CONNECT(MEDIAKEYS_SCHEMA_VOLUME_UP, _hash):
    case CONNECT(MEDIAKEYS_SCHEMA_VOLUME_UP_QUIET, _hash):
        this->audio_->do_sound_action(VOLUME_UP);
        break;
    case CONNECT(MEDIAKEYS_SCHEMA_POWER, _hash):
        this->do_shutdown();
        break;
    case CONNECT(MEDIAKEYS_SCHEMA_LOGOUT, _hash):
        this->do_logout();
        break;
    case CONNECT(MEDIAKEYS_SCHEMA_EJECT, _hash):
        this->do_eject();
        break;
    case CONNECT(MEDIAKEYS_SCHEMA_SCREENSAVER, _hash):
        this->do_screensaver();
        break;
    case CONNECT(MEDIAKEYS_SCHEMA_SHOW_DESKTOP, _hash):
        this->do_show_desktop();
        break;
    case CONNECT(MEDIAKEYS_SCHEMA_PANEL_KIRAN_MENU, _hash):
        this->do_panel_kiran_menu(xev);
        break;
    case CONNECT(MEDIAKEYS_SCHEMA_HOME, _hash):
        this->do_home();
        break;
    case CONNECT(MEDIAKEYS_SCHEMA_SEARCH, _hash):
        this->do_search();
        break;
    case CONNECT(MEDIAKEYS_SCHEMA_CONTROL_CENTER, _hash):
        this->do_control_center();
        break;
    case CONNECT(MEDIAKEYS_SCHEMA_CALCULATOR, _hash):
        this->do_calculator();
        break;
    case CONNECT(MEDIAKEYS_SCHEMA_EMAIL, _hash):
        this->do_appinfo_for_uri(APPINFO_URI_EMAIL);
        break;
    case CONNECT(MEDIAKEYS_SCHEMA_WWW, _hash):
        this->do_appinfo_for_uri(APPINFO_URI_WWW);
        break;
    case CONNECT(MEDIAKEYS_SCHEMA_HELP, _hash):
        this->do_appinfo_for_uri(APPINFO_URI_HELP);
        break;
    case CONNECT(MEDIAKEYS_SCHEMA_MEDIA, _hash):
        this->do_media();
        break;
    case CONNECT(MEDIAKEYS_SCHEMA_MAGNIFIER, _hash):
        this->do_screen_toggle(SCREEN_MAGNIFIER_ENABLED_KEY);
        break;
    case CONNECT(MEDIAKEYS_SCHEMA_SCREENREADER, _hash):
        this->do_screen_toggle(SCREEN_READER_ENABLED_KEY);
        break;
    case CONNECT(MEDIAKEYS_SCHEMA_ON_SCREEN_KEYBOARD, _hash):
        this->do_screen_toggle(SCREEN_KEYBOARD_ENABLED_KEY);
        break;
    default:
        break;
    }

    return false;
}

void MediaKeysAction::do_touchpad()
{
    if (!this->has_touchpad_)
    {
        OSDWindow::get_instance()->dialog_show(IMAGE_TOUCHPAD_DISABLED);
        return;
    }

    try
    {
        auto settings = Gio::Settings::create(TOUCHPAD_SCHEMA_ID);
        bool state = settings->get_boolean(TOUCHPAD_SCHEMA_TOUCHPAD_ENABLED);
        this->do_touchpad_osd(!state);
    }
    catch (const Glib::Error &e)
    {
        KLOG_WARNING_KEYBINDING("%s", e.what().c_str());
    }
}

void MediaKeysAction::do_touchpad_osd(bool state)
{
    if (!this->has_touchpad_)
    {
        OSDWindow::get_instance()->dialog_show(IMAGE_TOUCHPAD_DISABLED);
        return;
    }

    try
    {
        auto settings = Gio::Settings::create(TOUCHPAD_SCHEMA_ID);
        settings->set_boolean(TOUCHPAD_SCHEMA_TOUCHPAD_ENABLED, state);

        if (state)
        {
            OSDWindow::get_instance()->dialog_show(IMAGE_TOUCHPAD_ENABLED);
        }
        else
        {
            OSDWindow::get_instance()->dialog_show(IMAGE_TOUCHPAD_DISABLED);
        }
    }
    catch (const Glib::Error &e)
    {
        KLOG_WARNING_KEYBINDING("%s", e.what().c_str());
    }
}

void MediaKeysAction::do_shutdown()
{
    std::string cmdline = std::string("mate-session-save --shutdown-dialog");

    try
    {
        Glib::spawn_command_line_async(cmdline);
    }
    catch (const Glib::Error &e)
    {
        KLOG_WARNING_KEYBINDING("Exec command:%s fail: %s.", cmdline, e.what().c_str());
    }
}

void MediaKeysAction::do_logout()
{
    std::string cmdline = std::string("mate-session-save --logout-dialog");

    try
    {
        Glib::spawn_command_line_async(cmdline);
    }
    catch (const Glib::Error &e)
    {
        KLOG_WARNING_KEYBINDING("Exec command:%s fail: %s.", cmdline, e.what().c_str());
    }
}

void MediaKeysAction::do_screensaver()
{
    std::string cmdline;

    if (Glib::find_program_in_path("kiran-screensaver-command") != "")
    {
        cmdline = std::string("kiran-screensaver-command --lock");
    }
    else if (Glib::find_program_in_path("mate-screensaver-command") != "")
    {
        cmdline = std::string("mate-screensaver-command --lock");
    }
    else
    {
        cmdline = std::string("xscreensaver-command -lock");
    }

    try
    {
        Glib::spawn_command_line_async(cmdline);
    }
    catch (const Glib::Error &e)
    {
        KLOG_WARNING_KEYBINDING("Exec command:%s fail: %s.", cmdline, e.what().c_str());
    }
}

void MediaKeysAction::do_eject_action_cb(Glib::RefPtr<Gio::AsyncResult> &result, Glib::RefPtr<Gio::Drive> drive)
{
    try
    {
        drive->eject_finish(result);
        OSDWindow::get_instance()->dialog_show(IMAGE_MEDIA_EJECT);
    }
    catch (const Glib::Error &e)
    {
        KLOG_WARNING_KEYBINDING("%s", e.what().c_str());
    }
}

void MediaKeysAction::do_eject()
{
#define NO_SCORE 0
#define SCORE_CAN_EJECT 50
#define SCORE_HAS_MEDIA 100

    Glib::RefPtr<Gio::VolumeMonitor> volume_monitor = Gio::VolumeMonitor::get();
    Glib::ListHandle<Glib::RefPtr<Gio::Drive>> drives = volume_monitor->get_connected_drives();
    Glib::RefPtr<Gio::Drive> fav_drive;

    int score = NO_SCORE;

    // Find the best drive to eject
    for (auto drive : drives)
    {
        CONTINUE_IF_FALSE(drive->can_eject());

        CONTINUE_IF_FALSE(drive->is_media_removable());

        if (score < SCORE_CAN_EJECT)
        {
            fav_drive = drive;
            score = SCORE_CAN_EJECT;
        }

        CONTINUE_IF_FALSE(drive->has_media());

        if (score < SCORE_HAS_MEDIA)
        {
            fav_drive = drive;
            score = SCORE_HAS_MEDIA;
            break;
        }
    }

    if (fav_drive)
    {
        Gio::SlotAsyncReady res = sigc::bind(sigc::mem_fun(this, &MediaKeysAction::do_eject_action_cb), fav_drive);
        fav_drive->eject(res, Gio::MOUNT_UNMOUNT_FORCE);
    }
    else
    {
        KLOG_WARNING_KEYBINDING("There has not found suitable drives.");
    }
}

void MediaKeysAction::do_home()
{
    std::string home_path = Glib::shell_quote(Glib::get_home_dir());
    std::string cmdline = fmt::format("caja --no-desktop {0}", home_path);

    try
    {
        Glib::spawn_command_line_async(cmdline);
    }
    catch (const Glib::Error &e)
    {
        KLOG_WARNING_KEYBINDING("Exec command:%s fail: %s.", cmdline, e.what().c_str());
    }
}

void MediaKeysAction::do_search()
{
    std::string cmdline;

    if (Glib::find_program_in_path("beagle-search") != "")
    {
        cmdline = std::string("beagle-search");
    }
    else if (Glib::find_program_in_path("tracker-search-tool") != "")
    {
        cmdline = std::string("tracker-search-tool");
    }
    else
    {
        cmdline = std::string("mate-search-tool");
    }

    try
    {
        Glib::spawn_command_line_async(cmdline);
    }
    catch (const Glib::Error &e)
    {
        KLOG_WARNING_KEYBINDING("Exec command:%s fail: %s.", cmdline, e.what().c_str());
    }
}

void MediaKeysAction::do_control_center()
{
    std::string cmdline = std::string("kiran-control-panel");

    try
    {
        Glib::spawn_command_line_async(cmdline);
    }
    catch (const Glib::Error &e)
    {
        KLOG_WARNING_KEYBINDING("Exec command:%s fail: %s.", cmdline, e.what().c_str());
    }
}

void MediaKeysAction::do_calculator()
{
    std::string cmdline = std::string("kiran-calculator");

    try
    {
        Glib::spawn_command_line_async(cmdline);
    }
    catch (const Glib::Error &e)
    {
        KLOG_WARNING_KEYBINDING("Exec command:%s fail: %s.", cmdline, e.what().c_str());
    }
}

void MediaKeysAction::do_media()
{
    try
    {
        Glib::RefPtr<Gio::File> file;
        Glib::RefPtr<Gio::AppInfo> app = Gio::AppInfo::get_default_for_type(APPINFO_CONTEXT_MEDIA, false);
        if (app)
        {
            if (!app->launch(file))
            {
                KLOG_WARNING_KEYBINDING("Could not launch '%s'", APPINFO_CONTEXT_MEDIA);
            }
        }
        else
        {
            KLOG_WARNING_KEYBINDING("Could not find default application for '%s'", APPINFO_CONTEXT_MEDIA);
        }
    }
    catch (const Glib::Error &e)
    {
        KLOG_WARNING_KEYBINDING("Launch for %s fail: %s.", APPINFO_CONTEXT_MEDIA, e.what().c_str());
    }
}

void MediaKeysAction::do_appinfo_for_uri(const Glib::ustring &uri)
{
    try
    {
        Glib::RefPtr<Gio::File> file;
        Glib::RefPtr<Gio::AppInfo> app = Gio::AppInfo::get_default_for_uri_scheme(uri);
        if (app)
        {
            if (!app->launch(file))
            {
                KLOG_WARNING_KEYBINDING("Could not launch '%s'", uri.c_str());
            }
        }
        else
        {
            KLOG_WARNING_KEYBINDING("Could not find default application for '%s'", uri.c_str());
        }
    }
    catch (const Glib::Error &e)
    {
        KLOG_WARNING_KEYBINDING("Launch for uri:%s fail: %s.", uri, e.what().c_str());
    }
}

void MediaKeysAction::do_screen_toggle(const Glib::ustring &key)
{
    try
    {
        auto settings = Gio::Settings::create(DESKTOP_APP_SCHEMA_ID);
        bool state = settings->get_boolean(key);
        settings->set_boolean(key, !state);
    }
    catch (const Glib::Error &e)
    {
        KLOG_WARNING_KEYBINDING("Do screen toggle for key:%s fail: %s.", key, e.what().c_str());
    }
}

void MediaKeysAction::do_show_desktop()
{
    WnckScreen *screen = wnck_screen_get_default();
    if (screen)
    {
        bool mode = wnck_screen_get_showing_desktop(screen);
        wnck_screen_toggle_showing_desktop(screen, !mode);
    }
}

void MediaKeysAction::do_panel_kiran_menu(XEvent *xev)
{
    XClientMessageEvent ev;
    Display *dpy = xev->xkey.display;
    const char *atom_names[] = {"_MATE_PANEL_ACTION", "_MATE_PANEL_ACTION_KIRAN_MENU"};
    Atom atoms[G_N_ELEMENTS(atom_names)] = {0};

    XInternAtoms(dpy, const_cast<char **>(atom_names), G_N_ELEMENTS(atom_names), False, atoms);

    if (atoms[0] == None || atoms[1] == None)
    {
        KLOG_WARNING_KEYBINDING("Get panel kiran menu atom failed.");
        return;
    }

    ev.type = ClientMessage;
    ev.window = xev->xkey.window;
    ev.message_type = atoms[0];
    ev.format = 32;
    ev.data.l[0] = atoms[1];
    ev.data.l[1] = xev->xkey.time;

    XSendEvent(xev->xkey.display,
               xev->xkey.window,
               False,
               StructureNotifyMask,
               (XEvent *)&ev);
}

}  // namespace Kiran
