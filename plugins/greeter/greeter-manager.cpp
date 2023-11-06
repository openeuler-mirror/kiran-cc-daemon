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
 * Author:     songchuanfei <songchuanfei@kylinos.com.cn>
 */

#include "plugins/greeter/greeter-manager.h"

#include <errno.h>
#include <string.h>
#include <sys/stat.h>

#include "lib/base/base.h"

#define LIGHTDM_PROFILE_PATH "/usr/share/lightdm/lightdm.conf.d/99-kiran-greeter-login.conf"
#define LIGHTDM_GROUP_NAME "Seat:*"

#define GREETER_PROFILE_PATH "/etc/lightdm/kiran-greeter.conf"
#define GREETER_GROUP_NAME "Greeter"

#define KEY_AUTOLOGIN_USER "autologin-user"
#define KEY_AUTOLOGIN_DELAY "autologin-user-timeout"
#define KEY_AUTOLOGIN_SESSION "autologin-session"
#define DEFAULT_AUTOLOGIN_SESSION "kiran"

#define KEY_LIGHTDM_HIDE_USER_LIST "greeter-hide-users"
#define KEY_LIGHTDM_ENABLE_MANUAL_LOGIN "greeter-show-manual-login"

#define KEY_GREETER_ENABLE_MANUAL_LOGIN "enable-manual-login"
#define KEY_GREETER_HIDE_USER_LIST "user-list-hiding"

#define KEY_BACKGROUND_FILE "background-picture-uri"
#define KEY_SCALE_FACTOR "scale-factor"
#define KEY_ENABLE_SCALING "enable-scaling"

GreeterData::GreeterData() : scale_mode(GREETER_SCALING_MODE_AUTO),
                             autologin_delay(0),
                             scale_factor(1),
                             enable_manual_login(true),
                             hide_user_list(false),
                             autologin_user(""),
                             background_file("")
{
}

GreeterManager *GreeterManager::get_instance()
{
    static GreeterManager *prefs = nullptr;

    if (prefs == nullptr)
        prefs = new GreeterManager;
    return prefs;
}

GreeterManager::~GreeterManager()
{
    if (lightdm_settings != nullptr)
        delete lightdm_settings;

    if (greeter_settings != nullptr)
        delete greeter_settings;

    delete priv;
}

std::string GreeterManager::get_autologin_user() const
{
    g_return_val_if_fail(priv != nullptr, "");
    return priv->autologin_user.raw();
}

uint32_t GreeterManager::get_autologin_delay() const
{
    g_return_val_if_fail(priv != nullptr, 0);
    return priv->autologin_delay;
}

uint32_t GreeterManager::get_scale_factor() const
{
    return priv->scale_factor;
}

GreeterScalingMode GreeterManager::get_scale_mode() const
{
    return priv->scale_mode;
}

std::string GreeterManager::get_background_file() const
{
    return priv->background_file.raw();
}

bool GreeterManager::get_enable_manual_login() const
{
    return priv->enable_manual_login;
}

bool GreeterManager::get_hide_user_list() const
{
    return priv->hide_user_list;
}

void GreeterManager::set_autologin_user(const std::string &autologin_user)
{
    priv->autologin_user = autologin_user;
    lightdm_settings->set_string(LIGHTDM_GROUP_NAME, KEY_AUTOLOGIN_USER, autologin_user);
}

void GreeterManager::set_autologin_delay(uint32_t autologin_delay)
{
    priv->autologin_delay = autologin_delay;
    lightdm_settings->set_uint64(LIGHTDM_GROUP_NAME, KEY_AUTOLOGIN_DELAY, autologin_delay);
}

void GreeterManager::set_enable_manual_login(bool enable_manual_login)
{
    priv->enable_manual_login = enable_manual_login;
    lightdm_settings->set_boolean(LIGHTDM_GROUP_NAME, KEY_LIGHTDM_ENABLE_MANUAL_LOGIN, enable_manual_login);
}

void GreeterManager::set_hide_user_list(bool hide_user_list)
{
    priv->hide_user_list = hide_user_list;
    lightdm_settings->set_boolean(LIGHTDM_GROUP_NAME, KEY_LIGHTDM_HIDE_USER_LIST, hide_user_list);
}

void GreeterManager::set_scale_mode(GreeterScalingMode mode)
{
    Glib::ustring scale_mode;

    priv->scale_mode = mode;
    switch (mode)
    {
    case GREETER_SCALING_MODE_AUTO:
        scale_mode = "auto";
        break;
    case GREETER_SCALING_MODE_MANUAL:
        scale_mode = "manual";
        break;
    case GREETER_SCALING_MODE_DISABLE:
        scale_mode = "disable";
        break;
    default:
        g_return_if_reached();
    }
    greeter_settings->set_string(GREETER_GROUP_NAME, KEY_ENABLE_SCALING, scale_mode);
}

void GreeterManager::set_scale_factor(uint32_t scale_factor)
{
    priv->scale_factor = scale_factor;
    greeter_settings->set_uint64(GREETER_GROUP_NAME, KEY_SCALE_FACTOR, scale_factor);
}

void GreeterManager::set_background_file(const std::string &background_file)
{
    priv->background_file = background_file;
    greeter_settings->set_string(GREETER_GROUP_NAME, KEY_BACKGROUND_FILE, background_file);
}

void GreeterManager::init_settings_monitor()
{
    lightdm_conf = Gio::File::create_for_path(LIGHTDM_PROFILE_PATH);
    greeter_conf = Gio::File::create_for_path(GREETER_PROFILE_PATH);

    lightdm_monitor = lightdm_conf->monitor_file();
    greeter_monitor = greeter_conf->monitor_file();

    lightdm_monitor->signal_changed().connect(
        sigc::mem_fun(*this, &GreeterManager::on_profile_changed));
    greeter_monitor->signal_changed().connect(
        sigc::mem_fun(*this, &GreeterManager::on_profile_changed));
}

void GreeterManager::on_profile_changed(const Glib::RefPtr<Gio::File> &file,
                                        const Glib::RefPtr<Gio::File> &other_file,
                                        Gio::FileMonitorEvent event_type)
{
    GreeterData new_data;
    Glib::KeyFile *lightdm_settings_, *greeter_settings_;

    if (event_type != Gio::FILE_MONITOR_EVENT_CHANGED)
        return;

    KLOG_DEBUG_GREETER("file '%s' changed, event 0x%x",
                       file->get_path().c_str(),
                       (int)event_type);

    lightdm_settings_ = new Glib::KeyFile();
    greeter_settings_ = new Glib::KeyFile();

    if (!load_greeter_settings(&new_data, greeter_settings_))
    {
        KLOG_ERROR_GREETER("Failed to reload greeter settings");
        delete lightdm_settings_;
        delete greeter_settings_;
        return;
    }

    if (!load_lightdm_settings(&new_data, lightdm_settings_))
    {
        KLOG_ERROR_GREETER("Failed to reload lightdm settings");
        delete lightdm_settings_;
        delete greeter_settings_;
        return;
    }

    if (lightdm_settings != nullptr)
        delete lightdm_settings;

    if (greeter_settings != nullptr)
        delete greeter_settings;

    lightdm_settings = lightdm_settings_;
    greeter_settings = greeter_settings_;

    if (new_data.autologin_delay != priv->autologin_delay)
    {
        KLOG_DEBUG_GREETER("Autologin-delay changed from %u to %u", priv->autologin_delay, new_data.autologin_delay);
        priv->autologin_delay = new_data.autologin_delay;
        m_signal_autologin_delay_changed.emit();
    }

    if (new_data.autologin_user != priv->autologin_user)
    {
        KLOG_DEBUG_GREETER("Autologin-user changed from '%s' to '%s'", priv->autologin_user.c_str(),
                           new_data.autologin_user.c_str());
        priv->autologin_user = new_data.autologin_user;
        m_signal_autologin_user_changed.emit();
    }

    if (new_data.scale_mode != priv->scale_mode)
    {
        KLOG_DEBUG_GREETER("Scale-mode changed from %d to %d", priv->scale_mode, new_data.scale_mode);
        priv->scale_mode = new_data.scale_mode;
        m_signal_scale_mode_changed.emit();
    }

    if (new_data.background_file != priv->background_file)
    {
        KLOG_DEBUG_GREETER("Backgrond-file changed from '%s' to '%s'",
                           priv->background_file.c_str(),
                           new_data.background_file.c_str());
        priv->background_file = new_data.background_file;
        m_signal_background_file_changed.emit();
    }

    if (new_data.enable_manual_login != priv->enable_manual_login)
    {
        KLOG_DEBUG_GREETER("Enable-manual-login changed from %d to %d",
                           priv->enable_manual_login,
                           new_data.enable_manual_login);
        priv->enable_manual_login = new_data.enable_manual_login;
        m_signal_enable_manual_login_changed.emit();
    }

    if (new_data.hide_user_list != priv->hide_user_list)
    {
        KLOG_DEBUG_GREETER("Hide-user-list changed from %d to %d",
                           priv->hide_user_list,
                           new_data.hide_user_list);
        priv->hide_user_list = new_data.hide_user_list;
        m_signal_hide_user_list_changed.emit();
    }

    if (new_data.scale_factor != priv->scale_factor)
    {
        KLOG_DEBUG_GREETER("Scale-factor changed from %u to %u",
                           priv->scale_factor,
                           new_data.scale_factor);
        priv->scale_factor = new_data.scale_factor;
        m_signal_scale_factor_changed.emit();
    }
}

sigc::signal<void> GreeterManager::signal_autologin_delay_changed()
{
    return m_signal_autologin_delay_changed;
}

sigc::signal<void> GreeterManager::signal_autologin_user_changed()
{
    return m_signal_autologin_user_changed;
}

sigc::signal<void> GreeterManager::signal_scale_mode_changed()
{
    return m_signal_scale_mode_changed;
}

sigc::signal<void> GreeterManager::signal_scale_factor_changed()
{
    return m_signal_scale_factor_changed;
}

sigc::signal<void> GreeterManager::signal_background_file_changed()
{
    return m_signal_background_file_changed;
}

sigc::signal<void> GreeterManager::signal_enable_manual_login_changed()
{
    return m_signal_enable_manual_login_changed;
}

sigc::signal<void> GreeterManager::signal_hide_user_list_changed()
{
    return m_signal_hide_user_list_changed;
}

GreeterManager::GreeterManager() : lightdm_settings(nullptr),
                                   greeter_settings(nullptr)
{
    priv = new GreeterData;
    init_settings_monitor();
}

bool GreeterManager::settings_has_key(Glib::KeyFile *settings, const Glib::ustring &group, const Glib::ustring &key)
{
    try
    {
        if (settings == nullptr)
            return false;
        return settings->has_key(group, key);
    }
    catch (Glib::KeyFileError &e)
    {
        KLOG_WARNING_GREETER("Key '%s' not found: %s", key.c_str(), e.what().c_str());
        return false;
    }

    return true;
}

bool GreeterManager::load()
{
    *priv = GreeterData();

    if (greeter_settings != nullptr)
        delete greeter_settings;

    if (lightdm_settings != nullptr)
        delete lightdm_settings;

    try
    {
        greeter_settings = new Glib::KeyFile();
        lightdm_settings = new Glib::KeyFile();

        /*
         * lightdm配置文件优先于greeter配置文件，因此必须先加载
         * greeter配置文件，再加载lightdm配置文件，确保配置项可正常
         * 覆盖
         */
        if (!load_greeter_settings(priv, greeter_settings))
        {
            KLOG_ERROR_GREETER("Failed to load greeter settings");
            return false;
        }

        if (!load_lightdm_settings(priv, lightdm_settings))
        {
            KLOG_ERROR_GREETER("Failed to load lightdm settings");
            return false;
        }
    }
    catch (const Glib::Error &e)
    {
        KLOG_ERROR_GREETER("Loading Error: %s", e.what().c_str());
        return false;
    }

    return true;
}

bool GreeterManager::save()
{
    g_return_val_if_fail(lightdm_settings != nullptr, false);
    g_return_val_if_fail(greeter_settings != nullptr, false);

    try
    {
        lightdm_settings->save_to_file(LIGHTDM_PROFILE_PATH);
    }
    catch (const Glib::Error &e)
    {
        KLOG_WARNING_GREETER("Failed to save lightdm settings: %s", e.what().c_str());
        return false;
    }

    try
    {
        /*
         * enable_manual_login和hide_user_list将保存到lightdm的配置文件中
         */
        if (settings_has_key(greeter_settings, GREETER_GROUP_NAME, KEY_GREETER_ENABLE_MANUAL_LOGIN))
            greeter_settings->remove_key(GREETER_GROUP_NAME, KEY_GREETER_ENABLE_MANUAL_LOGIN);
        if (settings_has_key(greeter_settings, GREETER_GROUP_NAME, KEY_GREETER_HIDE_USER_LIST))
            greeter_settings->remove_key(GREETER_GROUP_NAME, KEY_GREETER_HIDE_USER_LIST);
        greeter_settings->save_to_file(GREETER_PROFILE_PATH);
    }
    catch (const Glib::Error &e)
    {
        KLOG_ERROR_GREETER("Failed to save greeter settings: %s", e.what().c_str());
        return false;
    }

    return true;
}

bool GreeterManager::load_greeter_settings(GreeterData *data, Glib::KeyFile *settings)
{
    bool success = true;
    Glib::KeyFile *tmp_settings = settings;

    g_return_val_if_fail(data != nullptr, false);

    try
    {
        if (tmp_settings == nullptr)
            tmp_settings = new Glib::KeyFile();

        if (!tmp_settings->load_from_file(GREETER_PROFILE_PATH, Glib::KEY_FILE_KEEP_COMMENTS))
        {
            KLOG_WARNING_GREETER("Failed to load configuration file '%s'", GREETER_PROFILE_PATH);
            success = false;
        }

        if (success)
        {
            if (settings_has_key(tmp_settings, GREETER_GROUP_NAME, KEY_AUTOLOGIN_USER))
            {
                auto user = tmp_settings->get_string(GREETER_GROUP_NAME, KEY_AUTOLOGIN_USER);
                data->autologin_user = user;
            }

            if (settings_has_key(tmp_settings, GREETER_GROUP_NAME, KEY_AUTOLOGIN_DELAY))
            {
                auto delay = tmp_settings->get_uint64(GREETER_GROUP_NAME, KEY_AUTOLOGIN_DELAY);
                data->autologin_delay = delay;
            }

            if (settings_has_key(tmp_settings, GREETER_GROUP_NAME, KEY_GREETER_ENABLE_MANUAL_LOGIN))
            {
                bool enable_manual_login = tmp_settings->get_boolean(GREETER_GROUP_NAME,
                                                                     KEY_GREETER_ENABLE_MANUAL_LOGIN);
                data->enable_manual_login = enable_manual_login;
            }

            if (settings_has_key(tmp_settings, GREETER_GROUP_NAME, KEY_GREETER_HIDE_USER_LIST))
            {
                bool hide_user_list = tmp_settings->get_boolean(GREETER_GROUP_NAME,
                                                                KEY_GREETER_HIDE_USER_LIST);
                data->hide_user_list = hide_user_list;
            }

            if (settings_has_key(tmp_settings, GREETER_GROUP_NAME, KEY_BACKGROUND_FILE))
            {
                auto background_file = tmp_settings->get_string(GREETER_GROUP_NAME, KEY_BACKGROUND_FILE);
                KLOG_DEBUG_GREETER("Background_file: %s", background_file.c_str());
                data->background_file = background_file;
            }

            if (settings_has_key(tmp_settings, GREETER_GROUP_NAME, KEY_ENABLE_SCALING))
            {
                auto enable_scaling = tmp_settings->get_string(GREETER_GROUP_NAME,
                                                               KEY_ENABLE_SCALING);
                KLOG_DEBUG_GREETER("Enable_scaling: %s", enable_scaling.c_str());
                if (enable_scaling == "auto")
                    data->scale_mode = GREETER_SCALING_MODE_AUTO;
                else if (enable_scaling == "manual")
                    data->scale_mode = GREETER_SCALING_MODE_MANUAL;
                else if (enable_scaling == "disable")
                    data->scale_mode = GREETER_SCALING_MODE_DISABLE;
                else
                {
                    KLOG_WARNING_GREETER("Invalid value '%s' for key '%s'", enable_scaling.c_str(), KEY_ENABLE_SCALING);
                    data->scale_mode = GREETER_SCALING_MODE_AUTO;
                }
            }

            if (settings_has_key(tmp_settings, GREETER_GROUP_NAME, KEY_SCALE_FACTOR))
            {
                auto scale_factor = tmp_settings->get_uint64(GREETER_GROUP_NAME, KEY_SCALE_FACTOR);
                if (scale_factor <= 1)
                    scale_factor = 1U;
                else
                    scale_factor = 2U;

                data->scale_factor = scale_factor;
            }
        }
    }
    catch (const Glib::Error &e)
    {
        KLOG_WARNING_GREETER("Failed to load configuration file '%s': %s", GREETER_PROFILE_PATH, e.what().c_str());
        success = false;
    }

    if (settings == nullptr)
        delete tmp_settings;

    return success;
}

bool GreeterManager::load_lightdm_settings(GreeterData *data, Glib::KeyFile *settings)
{
    bool success = true;
    Glib::KeyFile *tmp_settings = settings;

    g_return_val_if_fail(data != nullptr, false);

    try
    {
        if (tmp_settings == nullptr)
            tmp_settings = new Glib::KeyFile();

        if (!tmp_settings->load_from_file(LIGHTDM_PROFILE_PATH, Glib::KEY_FILE_KEEP_COMMENTS))
            success = false;

        if (success)
        {
            if (settings_has_key(tmp_settings, LIGHTDM_GROUP_NAME, KEY_AUTOLOGIN_USER))
            {
                auto user = tmp_settings->get_string(LIGHTDM_GROUP_NAME, KEY_AUTOLOGIN_USER);
                data->autologin_user = user;
            }

            if (settings_has_key(tmp_settings, LIGHTDM_GROUP_NAME, KEY_AUTOLOGIN_DELAY))
            {
                auto delay = tmp_settings->get_uint64(LIGHTDM_GROUP_NAME, KEY_AUTOLOGIN_DELAY);
                data->autologin_delay = delay;
            }

            // 自动登录用户需要默认配置 autologin-session=kiran
            if (!settings_has_key(tmp_settings, LIGHTDM_GROUP_NAME, KEY_AUTOLOGIN_SESSION))
            {
                tmp_settings->set_string(LIGHTDM_GROUP_NAME, KEY_AUTOLOGIN_SESSION, DEFAULT_AUTOLOGIN_SESSION);
            }

            if (settings_has_key(tmp_settings, LIGHTDM_GROUP_NAME, KEY_LIGHTDM_ENABLE_MANUAL_LOGIN))
            {
                bool enable_manual_login = tmp_settings->get_boolean(LIGHTDM_GROUP_NAME,
                                                                     KEY_LIGHTDM_ENABLE_MANUAL_LOGIN);
                data->enable_manual_login = enable_manual_login;
            }

            if (settings_has_key(tmp_settings, LIGHTDM_GROUP_NAME, KEY_LIGHTDM_HIDE_USER_LIST))
            {
                bool hide_user_list = tmp_settings->get_boolean(LIGHTDM_GROUP_NAME,
                                                                KEY_LIGHTDM_HIDE_USER_LIST);
                data->hide_user_list = hide_user_list;
            }
        }
    }
    catch (const Glib::Error &e)
    {
        KLOG_WARNING_GREETER("Failed to load configuration file '%s': %s",
                             LIGHTDM_PROFILE_PATH,
                             e.what().c_str());
        success = false;
    }

    if (settings == nullptr)
        delete tmp_settings;

    return success;
}
