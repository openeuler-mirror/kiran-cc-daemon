#include "greeter-settings-manager.h"

int main(void) {
    GreeterSettingsManager *prefs = GreeterSettingsManager::get_instance();
    if (prefs->load()) {
        g_message("load ok");
        g_message("autologin_user: %s", prefs->get_autologin_user().c_str());
        g_message("autologin_delay: %u", prefs->get_autologin_delay());
        g_message("scale mode: %d", prefs->get_scale_mode());
        g_message("background file: %s", prefs->get_background_file().c_str());
        g_message("enable_manual_login: %d", prefs->get_enable_manual_login());
        g_message("hide user list: %d", prefs->get_hide_user_list()); 
    }

    prefs->save();
    return 0;
}
