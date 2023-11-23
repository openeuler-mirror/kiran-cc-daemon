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

#ifdef KCC_SESSION_TYPE
#include <gdkmm/wrap_init.h>
#include <gtkmm.h>
#include <gtkmm/wrap_init.h>
#include <libnotify/notify.h>

#include "lib/display/EWMH.h"
#include "lib/osdwindow/osd-window.h"
#include "src/session-guarder.h"
#endif

#include <glib-unix.h>
#include <glib/gi18n.h>

#include "common.h"
#include "config.h"
#include "lib/base/base.h"
#include "lib/dbus/auth-manager.h"
#include "lib/iso/iso-translation.h"
#include "src/settings-manager.h"

#ifdef KCC_SESSION_TYPE
static void on_session_end()
{
    Kiran::SettingsManager::get_instance()->deactivate_plugins();
    gtk_main_quit();
}
#endif

int main(int argc, char* argv[])
{
#if defined KCC_SESSION_TYPE
    klog_gtk3_init(std::string(), "kylinsec-session", "kiran-cc-daemon", "kiran-session-daemon");
#elif defined KCC_SYSTEM_TYPE
    klog_gtk3_init(std::string(), "kylinsec-system", "kiran-cc-daemon", "kiran-system-daemon");
#endif
    Gio::init();

    setlocale(LC_ALL, "");
    bindtextdomain(GETTEXT_PACKAGE, KCC_LOCALEDIR);
    bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
    textdomain(GETTEXT_PACKAGE);

    Glib::OptionContext context;
    Glib::OptionGroup group("kiran-cc-daemon", "kiran-cc-daemon option group");

    // version
    Glib::OptionEntry version_entry;
    version_entry.set_long_name("version");
    version_entry.set_flags(Glib::OptionEntry::FLAG_NO_ARG);
    version_entry.set_description(N_("Output version infomation and exit"));

    group.add_entry(version_entry, [](const Glib::ustring& option_name, const Glib::ustring& value, bool has_value) -> bool
                    {
                        g_print("kiran-cc-daemon: 2.0\n");
                        return true;
                    });

    group.set_translation_domain(GETTEXT_PACKAGE);
    context.set_main_group(group);

    try
    {
        context.parse(argc, argv);
    }
    catch (const Glib::Exception& e)
    {
        KLOG_WARNING("%s", e.what().c_str());
        return EXIT_FAILURE;
    }

#if defined KCC_SYSTEM_TYPE
    auto loop = Glib::MainLoop::create();
#elif defined KCC_SESSION_TYPE
    auto app = Gtk::Application::create(CC_DAEMON_DBUS_NAME);
    gdk_set_allowed_backends("x11");
    gtk_init(NULL, NULL);
    Gdk::wrap_init();
    Gtk::wrap_init();
    notify_init(_("Control center"));
    Kiran::EWMH::global_init();

#endif

    Kiran::AuthManager::global_init();
    Kiran::ISOTranslation::global_init();
    Kiran::SettingsManager::global_init();

#if defined KCC_SESSION_TYPE
    Kiran::SessionGuarder::global_init();
    Kiran::SessionGuarder::get_instance()->signal_session_end().connect(&on_session_end);
    Kiran::OSDWindow::global_init();
#endif

#if defined KCC_SYSTEM_TYPE
    loop->run();
#elif defined KCC_SESSION_TYPE
    gtk_main();
#endif

#ifdef KCC_SESSION_TYPE
    Kiran::OSDWindow::global_deinit();
    Kiran::SessionGuarder::global_deinit();
#endif

    Kiran::SettingsManager::global_deinit();
    Kiran::ISOTranslation::global_deinit();
    Kiran::AuthManager::global_deinit();

#ifdef KCC_SESSION_TYPE
    Kiran::EWMH::global_deinit();
    notify_uninit();
#endif
    return 0;
}
