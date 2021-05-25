/**
 * @file          /kiran-cc-daemon/src/main.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#ifdef KCC_SESSION_TYPE
#include <gdkmm/wrap_init.h>
#include <gtkmm.h>
#include <gtkmm/wrap_init.h>
#include <libnotify/notify.h>

#include "lib/display/EWMH.h"
#include "src/session-guarder.h"
#endif

#include <glib-unix.h>
#include <glib/gi18n.h>
#include <zlog_ex.h>

#include "config.h"
#include "lib/base/base.h"
#include "lib/dbus/auth-manager.h"
#include "lib/iso/iso-translation.h"
#include "src/settings-manager.h"

#ifdef KCC_SESSION_TYPE
static void on_session_end()
{
    gtk_main_quit();
}
#endif

int main(int argc, char* argv[])
{
#if defined KCC_SESSION_TYPE
    dzlog_init_ex(NULL, "kylinsec-session", "kiran-cc-daemon", "kiran-session-daemon");
#elif defined KCC_SYSTEM_TYPE
    dzlog_init_ex(NULL, "kylinsec-system", "kiran-cc-daemon", "kiran-system-daemon");
#endif
    Gio::init();
    Kiran::Log::global_init();

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

    group.add_entry(version_entry, [](const Glib::ustring& option_name, const Glib::ustring& value, bool has_value) -> bool {
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
        LOG_WARNING("%s", e.what().c_str());
        return EXIT_FAILURE;
    }

#if defined KCC_SYSTEM_TYPE
    auto loop = Glib::MainLoop::create();
#elif defined KCC_SESSION_TYPE
    gtk_init(NULL, NULL);
    Gdk::wrap_init();
    Gtk::wrap_init();
    notify_init(_("Control center"));
    Kiran::EWMH::global_init();
    Kiran::SessionGuarder::global_init();
    Kiran::SessionGuarder::get_instance()->signal_session_end().connect(&on_session_end);

#endif

    Kiran::AuthManager::global_init();
    Kiran::ISOTranslation::global_init();
    Kiran::SettingsManager::global_init();

#if defined KCC_SYSTEM_TYPE
    loop->run();
#elif defined KCC_SESSION_TYPE
    gtk_main();
    Kiran::SessionGuarder::global_deinit();
    Kiran::EWMH::global_deinit();
    notify_uninit();
#endif
    return 0;
}
