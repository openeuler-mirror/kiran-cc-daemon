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

#include <glib/gi18n.h>
#include <gtk3-log-i.h>
#include <gtkmm/main.h>

#include "config.h"
#include "plugins/power/tray/power-tray.h"

#define KIRAN_POWER_STATUS_ICON_DBUS_NAME "com.kylinsec.Kiran.PowerStatusIcon"

int main(int argc, char* argv[])
{
    klog_gtk3_init(std::string(), "kylinsec-session", "kiran-cc-daemon", "kiran-power-status-icon");

    setlocale(LC_ALL, "");
    bindtextdomain("kiran-power-status-icon", KCC_LOCALEDIR);
    bind_textdomain_codeset("kiran-power-status-icon", "UTF-8");
    textdomain("kiran-power-status-icon");

    gtk_init(&argc, &argv);
    Gtk::Main::init_gtkmm_internals();

    auto app = Gio::Application::create(KIRAN_POWER_STATUS_ICON_DBUS_NAME);

    try
    {
        app->register_application();

        if (app->is_remote())
        {
            KLOG_WARNING_POWER("The program is already running, exiting");
            return EXIT_SUCCESS;
        }
    }
    catch (const Glib::Error& e)
    {
        KLOG_WARNING_POWER("%s", e.what().c_str());
        return EXIT_FAILURE;
    }

    Kiran::PowerTray::global_init();
    gtk_main();
    Kiran::PowerTray::global_deinit();
    return EXIT_SUCCESS;
}