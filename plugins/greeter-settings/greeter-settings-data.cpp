#include "greeter-settings-data.h"

GreeterSettingsData::GreeterSettingsData() :
    scale_mode(GREETER_SETTINGS_SCALING_MODE_AUTO),
    autologin_delay(0),
    scale_factor(1),
    enable_manual_login(true),
    hide_user_list(false),
    autologin_user(""),
    background_file("")
{

}

GreeterSettingsData::GreeterSettingsData(GreeterSettingsData &data_) 
{
    autologin_user = data_.autologin_user;
    autologin_delay = data_.autologin_delay;
    background_file = data_.background_file;
    enable_manual_login = data_.enable_manual_login;
    hide_user_list = data_.hide_user_list;
    scale_mode = data_.scale_mode;
    scale_factor = data_.scale_factor;
}
