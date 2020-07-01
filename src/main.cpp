/*
 * @Author       : tangjie02
 * @Date         : 2020-05-29 15:38:08
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-06-30 20:24:37
 * @Description  : 
 * @FilePath     : /kiran-system-daemon/src/main.cpp
 */

#include "lib/log.h"
#include "src/settings-manager.h"

int main()
{
    Kiran::Log::global_init();
    Kiran::SettingsManager::global_init();

    // bindtextdomain(GETTEXT_PACKAGE, MATE_SETTINGS_LOCALEDIR);
    // bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
    // textdomain(GETTEXT_PACKAGE);
    // setlocale(LC_ALL, "");
    return 0;
}