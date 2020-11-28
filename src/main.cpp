/*
 * @Author       : tangjie02
 * @Date         : 2020-05-29 15:38:08
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-11-28 10:59:01
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/src/main.cpp
 */

#ifdef KCC_SESSION_TYPE
#include <gdkmm.h>
#include <gdkmm/wrap_init.h>

#include "lib/display/EWMH.h"
#include "src/session-guarder.h"
#endif

#include <glib-unix.h>
#include <glib/gi18n.h>

#include "lib/base/base.h"
#include "lib/dbus/auth-manager.h"
#include "lib/iso/iso-translation.h"
#include "src/settings-manager.h"

class ScreenLogger : public Kiran::ILogger
{
public:
    void write_log(const char* buff, uint32_t len)
    {
        g_print("%s", buff);
    }
};

int main(int argc, char* argv[])
{
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

    // verbose
    Glib::OptionEntry verbose_entry;
    verbose_entry.set_long_name("verbose");
    verbose_entry.set_flags(Glib::OptionEntry::FLAG_NO_ARG);
    verbose_entry.set_description(N_("Set log level to debug."));

    // show-log
    Glib::OptionEntry show_log_entry;
    show_log_entry.set_long_name("show-log");
    show_log_entry.set_flags(Glib::OptionEntry::FLAG_NO_ARG);
    show_log_entry.set_description(N_("Print log to screen."));

    group.add_entry(version_entry, [](const Glib::ustring& option_name, const Glib::ustring& value, bool has_value) -> bool {
        g_print("kiran-cc-daemon: 2.0\n");
        return false;
    });

    group.add_entry(verbose_entry, [](const Glib::ustring& option_name, const Glib::ustring& value, bool has_value) -> bool {
        Kiran::Log::get_instance()->set_log_level(G_LOG_LEVEL_DEBUG);
        return true;
    });

    group.add_entry(show_log_entry, [](const Glib::ustring& option_name, const Glib::ustring& value, bool has_value) -> bool {
        Kiran::Log::get_instance()->set_logger(new ScreenLogger());
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

    auto loop = Glib::MainLoop::create();

#ifdef KCC_SESSION_TYPE
    gdk_init(NULL, NULL);
    Gdk::wrap_init();
    Kiran::EWMH::global_init();
    Kiran::SessionGuarder::global_init(loop);
#endif

    Kiran::AuthManager::global_init();
    Kiran::ISOTranslation::global_init();
    Kiran::SettingsManager::global_init();

    loop->run();
    return 0;
}
