#include "greeter-settings-manager.h"
#include "log.h"

#define LIGHTDM_PROFILE_PATH "/etc/lightdm/lightdm.conf"
#define LIGHTDM_GROUP_NAME "Seat:*"

#define GREETER_PROFILE_PATH "/etc/lightdm/lightdm-kiran-greeter.conf"
#define GREETER_GROUP_NAME "Greeter"

#define KEY_AUTOLOGIN_USER "autologin-user"
#define KEY_AUTOLOGIN_DELAY "autologin-user-timeout"

#define KEY_LIGHTDM_HIDE_USER_LIST "greeter-hide-users"
#define KEY_LIGHTDM_ENABLE_MANUAL_LOGIN "greeter-show-manual-login"

#define KEY_GREETER_ENABLE_MANUAL_LOGIN "enable-manual-login"
#define KEY_GREETER_HIDE_USER_LIST "user-list-hiding"

#define KEY_BACKGROUND_FILE "background-picture-uri"
#define KEY_SCALE_FACTOR "scale-factor"
#define KEY_ENABLE_SCALING "enable-scaling"

GreeterSettingsManager *GreeterSettingsManager::get_instance()
{
    static GreeterSettingsManager *prefs = nullptr;

    if (prefs == nullptr)
        prefs = new GreeterSettingsManager;
    return prefs;
}

GreeterSettingsManager::~GreeterSettingsManager()
{
    if (lightdm_settings != nullptr)
        delete lightdm_settings;

    if (lightdm_settings != nullptr)
        delete lightdm_settings;
}

std::string GreeterSettingsManager::get_autologin_user() const
{
    return m_autologin_user.raw();
}

uint32_t GreeterSettingsManager::get_autologin_delay() const
{
    return m_autologin_delay;
}

uint32_t GreeterSettingsManager::get_scale_factor() const
{
    return m_scale_factor;
}

GreeterSettingsManager::GreeterScalingMode GreeterSettingsManager::get_scale_mode() const
{
    return m_scale_mode;
}

std::string GreeterSettingsManager::get_background_file() const
{
    return m_background_file.raw();
}

bool GreeterSettingsManager::get_enable_manual_login() const
{
    return m_enable_manual_login;
}

bool GreeterSettingsManager::get_hide_user_list() const
{
    return m_hide_user_list;
}

void GreeterSettingsManager::set_autologin_user(const std::string &autologin_user)
{
    m_autologin_user = autologin_user;
    lightdm_settings->set_string(LIGHTDM_GROUP_NAME, KEY_AUTOLOGIN_USER, m_autologin_user);
}

void GreeterSettingsManager::set_autologin_delay(uint32_t autologin_delay)
{
    m_autologin_delay = autologin_delay;
    lightdm_settings->set_uint64(LIGHTDM_GROUP_NAME, KEY_AUTOLOGIN_DELAY, m_autologin_delay);
}

void GreeterSettingsManager::set_enable_manual_login(bool enable_manual_login)
{
    m_enable_manual_login = enable_manual_login;
    lightdm_settings->set_boolean(LIGHTDM_GROUP_NAME, KEY_LIGHTDM_ENABLE_MANUAL_LOGIN, m_enable_manual_login);
}

void GreeterSettingsManager::set_hide_user_list(bool hide_user_list)
{
    m_hide_user_list = hide_user_list;
    lightdm_settings->set_boolean(LIGHTDM_GROUP_NAME, KEY_LIGHTDM_HIDE_USER_LIST, m_hide_user_list);
}

void GreeterSettingsManager::set_scale_mode(GreeterScalingMode mode)
{
    Glib::ustring scale_mode;

    m_scale_mode = mode;
    switch (mode)
    {
    case SCALING_AUTO:
        scale_mode = "auto";
        break;
    case SCALING_MANUAL:
        scale_mode = "manual";
        break;
    case SCALING_DISABLE:
        scale_mode = "disable";
        break;
    default:
        g_return_if_reached();
    }
    greeter_settings->set_string(GREETER_GROUP_NAME, KEY_ENABLE_SCALING, scale_mode);
}

void GreeterSettingsManager::set_scale_factor(uint32_t scale_factor)
{
    m_scale_factor = scale_factor;
    greeter_settings->set_uint64(GREETER_GROUP_NAME, KEY_SCALE_FACTOR, m_scale_factor);
}

void GreeterSettingsManager::set_background_file(const std::string &background_file)
{
    m_background_file = background_file;
    greeter_settings->set_string(GREETER_GROUP_NAME, KEY_BACKGROUND_FILE, m_background_file);
}

GreeterSettingsManager::GreeterSettingsManager() : lightdm_settings(nullptr),
                               greeter_settings(nullptr),
                               m_autologin_delay(),
                               m_autologin_user(""),
                               m_background_file(""),
                               m_enable_manual_login(true),
                               m_hide_user_list(false),
                               m_scale_mode(GreeterSettingsManager::SCALING_AUTO),
                               m_scale_factor(1)
{
}

bool GreeterSettingsManager::settings_has_key(Glib::KeyFile *settings, const Glib::ustring &group, const Glib::ustring &key)
{
    if (settings == nullptr)
        return false;

    try
    {
        return settings->has_key(group, key);
    }
    catch (Glib::KeyFileError &e)
    {
        LOG_WARNING("key '%s' not found: %s", key.c_str(), e.what().c_str());
        return false;
    }

    return true;
}

bool GreeterSettingsManager::load()
{
    GError *error = nullptr;
    Glib::ustring value;

    if (!load_lightdm_settings())
    {
        LOG_WARNING("Failed to load lightdm settings");
        return false;
    }

    if (!load_greeter_settings())
    {
        LOG_WARNING("Failed to load greeter settings");
        return false;
    }

    try
    {
        if (settings_has_key(lightdm_settings, LIGHTDM_GROUP_NAME, KEY_AUTOLOGIN_USER))
        {
            auto user = lightdm_settings->get_string(LIGHTDM_GROUP_NAME, KEY_AUTOLOGIN_USER);
            set_autologin_user(user);
        }
        else if (settings_has_key(greeter_settings, LIGHTDM_GROUP_NAME, KEY_AUTOLOGIN_USER))
        {
            auto user = greeter_settings->get_string(LIGHTDM_GROUP_NAME, KEY_AUTOLOGIN_USER);
            set_autologin_user(user);
        }
        else
            LOG_WARNING("settings '%s' not found, use default value", KEY_AUTOLOGIN_USER);

        if (settings_has_key(lightdm_settings, LIGHTDM_GROUP_NAME, KEY_AUTOLOGIN_DELAY))
        {
            auto delay = lightdm_settings->get_uint64(LIGHTDM_GROUP_NAME, KEY_AUTOLOGIN_DELAY);
            set_autologin_delay(delay);
        }
        else if (settings_has_key(greeter_settings, LIGHTDM_GROUP_NAME, KEY_AUTOLOGIN_DELAY))
        {
            auto delay = greeter_settings->get_uint64(LIGHTDM_GROUP_NAME, KEY_AUTOLOGIN_DELAY);
            set_autologin_delay(delay);
        }
        else
            LOG_WARNING("settings '%s' not found, use default value", KEY_AUTOLOGIN_DELAY);

        if (settings_has_key(lightdm_settings, LIGHTDM_GROUP_NAME, KEY_LIGHTDM_ENABLE_MANUAL_LOGIN))
        {
            bool enable_manual_login = lightdm_settings->get_boolean(LIGHTDM_GROUP_NAME,
                                                                     KEY_LIGHTDM_ENABLE_MANUAL_LOGIN);
            set_enable_manual_login(enable_manual_login);
        }
        else if (settings_has_key(greeter_settings, GREETER_GROUP_NAME, KEY_GREETER_ENABLE_MANUAL_LOGIN))
        {
            bool enable_manual_login = greeter_settings->get_boolean(GREETER_GROUP_NAME,
                                                                     KEY_GREETER_ENABLE_MANUAL_LOGIN);
            set_enable_manual_login(enable_manual_login);
        }
        else
        {
            LOG_WARNING("settings 'enable_manual_login' not found, use default value");
        }

        if (settings_has_key(lightdm_settings, LIGHTDM_GROUP_NAME, KEY_LIGHTDM_HIDE_USER_LIST))
        {
            bool hide_user_list = lightdm_settings->get_boolean(LIGHTDM_GROUP_NAME,
                                                                KEY_LIGHTDM_HIDE_USER_LIST);

            set_hide_user_list(hide_user_list);
        }
        else if (settings_has_key(greeter_settings, GREETER_GROUP_NAME, KEY_GREETER_HIDE_USER_LIST))
        {
            bool hide_user_list = greeter_settings->get_boolean(GREETER_GROUP_NAME,
                                                                KEY_GREETER_HIDE_USER_LIST);
            set_hide_user_list(hide_user_list);
        }
        else
        {
            LOG_WARNING("settings 'hide_user_list' not found, use default value");
        }

        if (settings_has_key(greeter_settings, GREETER_GROUP_NAME, KEY_BACKGROUND_FILE))
        {
            auto background_file = greeter_settings->get_string(GREETER_GROUP_NAME, KEY_BACKGROUND_FILE);
            LOG_DEBUG("background_file: %s", background_file.c_str());
            set_background_file(background_file);
        }

        if (settings_has_key(greeter_settings, GREETER_GROUP_NAME, KEY_ENABLE_SCALING))
        {
            auto enable_scaling = greeter_settings->get_string(GREETER_GROUP_NAME,
                                                               KEY_ENABLE_SCALING);
            LOG_DEBUG("enable_scaling: %s", enable_scaling.c_str());
            if (enable_scaling == "auto")
                set_scale_mode(GreeterSettingsManager::SCALING_AUTO);
            else if (enable_scaling == "manual")
                set_scale_mode(GreeterSettingsManager::SCALING_MANUAL);
            else if (enable_scaling == "disable")
                set_scale_mode(GreeterSettingsManager::SCALING_DISABLE);
            else
                LOG_WARNING("Invalid value '%s' for key '%s'", enable_scaling, KEY_ENABLE_SCALING);
        }

        if (settings_has_key(greeter_settings, GREETER_GROUP_NAME, KEY_SCALE_FACTOR))
        {
            auto scale_factor = greeter_settings->get_uint64(GREETER_GROUP_NAME, KEY_SCALE_FACTOR);
            if (scale_factor <= 1)
                scale_factor = 3U;
            else
                scale_factor = 2U;
            set_scale_factor(scale_factor);
        }
    }
    catch (const Glib::Error &e)
    {
        LOG_ERROR("Loading Error: %s", e.what().c_str());
        return false;
    }

    return true;
}

bool GreeterSettingsManager::save()
{
    g_return_val_if_fail(lightdm_settings != nullptr, false);
    g_return_val_if_fail(greeter_settings != nullptr, false);

    try
    {
        lightdm_settings->save_to_file(LIGHTDM_PROFILE_PATH);
    }
    catch (const Glib::Error &e)
    {
        LOG_WARNING("Failed to save lightdm settings: %s", e.what().c_str());
        return false;
    }

    try
    {
        /*
         * enable_manual_login和hide_user_list将保存到lightdm的配置文件中
         */
        greeter_settings->remove_key(GREETER_GROUP_NAME, KEY_GREETER_ENABLE_MANUAL_LOGIN);
        greeter_settings->remove_key(GREETER_GROUP_NAME, KEY_GREETER_HIDE_USER_LIST);
        greeter_settings->save_to_file(GREETER_PROFILE_PATH);
    }
    catch (const Glib::Error &e)
    {
        LOG_ERROR("Failed to save greeter settings: %s", e.what().c_str());
        return false;
    }

    return true;
}

bool GreeterSettingsManager::load_greeter_settings()
{
    bool success = true;

    if (greeter_settings != nullptr)
        delete greeter_settings;

    greeter_settings = new Glib::KeyFile();
    try
    {
        if (!greeter_settings->load_from_file(GREETER_PROFILE_PATH, Glib::KEY_FILE_KEEP_COMMENTS))
        {
            LOG_CRITICAL("Failed to load configuration file '%s'", GREETER_PROFILE_PATH);
            success = false;
        }
    }
    catch (const Glib::Error &e)
    {
        LOG_CRITICAL("Failed to load configuration file '%s': %s",
                   GREETER_PROFILE_PATH, e.what().c_str());
        success = false;
    }

    if (!success)
    {
        delete greeter_settings;
        greeter_settings = nullptr;
    }

    return success;
}

bool GreeterSettingsManager::load_lightdm_settings()
{
    bool success = true;

    if (lightdm_settings != nullptr)
        delete lightdm_settings;

    lightdm_settings = new Glib::KeyFile();
    try
    {
        if (!lightdm_settings->load_from_file(LIGHTDM_PROFILE_PATH, Glib::KEY_FILE_KEEP_COMMENTS))
        {
            success = false;
        }
    }
    catch (const Glib::Error &e)
    {
        LOG_CRITICAL("Failed to load configuration file '%s': %s",
                   LIGHTDM_PROFILE_PATH,
                   e.what().c_str());
        success = false;
    }

    if (!success)
    {

        delete lightdm_settings;
        lightdm_settings = nullptr;
    }

    return success;
}
