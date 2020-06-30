/*
 * @Author       : tangjie02
 * @Date         : 2020-05-29 15:38:08
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-06-18 19:55:10
 * @Description  : 
 * @FilePath     : /kiran-session-daemon/src/main.cpp
 */

#include "src/log.h"
#include "src/settings-manager.h"

class TestLogger : public Kiran::ILogger
{
public:
    void write_log(const char *buff, uint32_t len)
    {
        printf("%s\n", buff);
    }
};

int main()
{
    Kiran::Log::global_init();
    Kiran::Log::get_instance()->set_logger(new TestLogger());
    Kiran::SettingsManager::global_init();

    Kiran::SettingsManager::global_deinit();

    // bindtextdomain(GETTEXT_PACKAGE, MATE_SETTINGS_LOCALEDIR);
    // bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
    // textdomain(GETTEXT_PACKAGE);
    // setlocale(LC_ALL, "");
    return 0;
}