/**
 * @file          /kiran-cc-daemon/plugins/greeter/greeter-dbus.cpp
 * @brief         
 * @author        yangxiaoqing <yangxiaoqing@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#include "plugins/greeter/greeter-dbus.h"

#include <glib/gstdio.h>
#include <pwd.h>
#include <unistd.h>

#include <cinttypes>
#include <cstdint>

#define GREETER_NEW_INTERFACE
#include "greeter_i.h"
#include "lib/base/base.h"
#include "lib/dbus/dbus.h"

namespace Kiran
{
#define AUTH_SET_LOGIN_OPTION "com.kylinsec.kiran.system-daemon.greeter.set-login-option"

#define GREETERSETTINGSDBUS_SET_ONE_PROP_AUTH(fun, callback, auth, type1)                                                     \
    void GreeterDBus::fun(type1 value,                                                                                        \
                          MethodInvocation &invocation)                                                                       \
    {                                                                                                                         \
        SETTINGS_PROFILE("");                                                                                                 \
                                                                                                                              \
        AuthManager::get_instance()->start_auth_check(AUTH_SET_LOGIN_OPTION,                                                  \
                                                      TRUE,                                                                   \
                                                      invocation.getMessage(),                                                \
                                                      std::bind(&GreeterDBus::callback, this, std::placeholders::_1, value)); \
                                                                                                                              \
        return;                                                                                                               \
    }

#define GREETERSETTINGSDBUS_SET_TWO_PROP_AUTH(fun, callback, auth, type1, type2)                                                       \
    void GreeterDBus::fun(type1 value1, type2 value2, MethodInvocation &invocation)                                                    \
    {                                                                                                                                  \
        SETTINGS_PROFILE("");                                                                                                          \
                                                                                                                                       \
        AuthManager::get_instance()->start_auth_check(AUTH_SET_LOGIN_OPTION,                                                           \
                                                      TRUE,                                                                            \
                                                      invocation.getMessage(),                                                         \
                                                      std::bind(&GreeterDBus::callback, this, std::placeholders::_1, value1, value2)); \
                                                                                                                                       \
        return;                                                                                                                        \
    }

GREETERSETTINGSDBUS_SET_ONE_PROP_AUTH(SetBackground, change_background_file_authorized_cb, AUTH_SET_LOGIN_OPTION, const Glib::ustring &)
GREETERSETTINGSDBUS_SET_ONE_PROP_AUTH(SetAutologinUser, change_auto_login_user_authorized_cb, AUTH_SET_LOGIN_OPTION, const Glib::ustring &)
GREETERSETTINGSDBUS_SET_ONE_PROP_AUTH(SetAutologinTimeout, change_auto_login_timeout_authorized_cb, AUTH_SET_LOGIN_OPTION, uint64_t)
GREETERSETTINGSDBUS_SET_ONE_PROP_AUTH(SetHideUserList, change_hide_user_list_authorized_cb, AUTH_SET_LOGIN_OPTION, bool)
GREETERSETTINGSDBUS_SET_ONE_PROP_AUTH(SetAllowManualLogin, change_allow_manual_login_authorized_cb, AUTH_SET_LOGIN_OPTION, bool)
GREETERSETTINGSDBUS_SET_TWO_PROP_AUTH(SetScaleMode, change_scale_mode_authorized_cb, AUTH_SET_LOGIN_OPTION, uint16_t, uint16_t)

#define USER_PROP_SET_HANDLER(prop, type)                                  \
    bool GreeterDBus::prop##_setHandler(type value)                        \
    {                                                                      \
        SETTINGS_PROFILE("value: %s.", fmt::format("{0}", value).c_str()); \
        this->prop##_ = value;                                             \
        return true;                                                       \
    }

USER_PROP_SET_HANDLER(background, const Glib::ustring &)
USER_PROP_SET_HANDLER(autologin_user, const Glib::ustring &)
USER_PROP_SET_HANDLER(autologin_timeout, guint64)
USER_PROP_SET_HANDLER(allow_manual_login, bool)
USER_PROP_SET_HANDLER(hide_user_list, bool)
USER_PROP_SET_HANDLER(scale_mode, guint16)
USER_PROP_SET_HANDLER(scale_factor, guint16)

GreeterDBus::GreeterDBus()
{
    autologin_timeout_ = 0;
    allow_manual_login_ = false;
    hide_user_list_ = false;
    scale_mode_ = 0;
    scale_factor_ = 0;
    dbus_connect_id_ = 0;
    object_register_id_ = 0;

    m_prefs = GreeterManager::get_instance();
}

GreeterDBus::~GreeterDBus()
{
    if (this->dbus_connect_id_)
    {
        Gio::DBus::unown_name(this->dbus_connect_id_);
    }

    if (this->m_reloadConn)
    {
        this->m_reloadConn.disconnect();
    }
}

GreeterDBus *GreeterDBus::m_instance = nullptr;
void GreeterDBus::global_init()
{
    m_instance = new GreeterDBus();
    m_instance->init();
}

void GreeterDBus::init()
{
    SETTINGS_PROFILE("");

    /*Connect to the system bus and acquire our name*/
    this->dbus_connect_id_ = Gio::DBus::own_name(Gio::DBus::BUS_TYPE_SYSTEM,
                                                 GREETER_DBUS_NAME,
                                                 sigc::mem_fun(this, &GreeterDBus::on_bus_acquired),
                                                 sigc::mem_fun(this, &GreeterDBus::on_name_acquired),
                                                 sigc::mem_fun(this, &GreeterDBus::on_name_lost));

    m_prefs->signal_autologin_delay_changed().connect(sigc::mem_fun(this, &GreeterDBus::on_autologin_delay_changed));
    m_prefs->signal_autologin_user_changed().connect(sigc::mem_fun(this, &GreeterDBus::on_autologin_user_changed));
    m_prefs->signal_background_file_changed().connect(sigc::mem_fun(this, &GreeterDBus::on_background_file_changed));
    m_prefs->signal_enable_manual_login_changed().connect(sigc::mem_fun(this, &GreeterDBus::on_enable_manual_login_changed));
    m_prefs->signal_hide_user_list_changed().connect(sigc::mem_fun(this, &GreeterDBus::on_hide_user_list_changed));
    m_prefs->signal_scale_factor_changed().connect(sigc::mem_fun(this, &GreeterDBus::on_scale_factor_changed));
    m_prefs->signal_scale_mode_changed().connect(sigc::mem_fun(this, &GreeterDBus::on_scale_mode_changed));

    reload_greeter_settings();
}

void GreeterDBus::change_background_file_authorized_cb(MethodInvocation invocation, Glib::ustring file_path)
{
    SETTINGS_PROFILE("file_path: %s", file_path.c_str());

    if (this->background_get() != file_path)
    {
        m_prefs->set_background_file(file_path);
        if (!m_prefs->save())
        {
            DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_GREETER_SYNC_TO_FILE_FAILED_1);
        }

        this->background_set(file_path);
    }
    invocation.ret();
}

void GreeterDBus::change_auto_login_user_authorized_cb(MethodInvocation invocation, Glib::ustring user_name)
{
    SETTINGS_PROFILE("autologin_user: %s", user_name.c_str());

    if (this->autologin_user_get() != user_name)
    {
        m_prefs->set_autologin_user(user_name);
        if (!m_prefs->save())
        {
            DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_GREETER_SYNC_TO_FILE_FAILED_2);
        }

        this->autologin_user_set(user_name);
    }

    invocation.ret();
}

void GreeterDBus::change_auto_login_timeout_authorized_cb(MethodInvocation invocation, guint64 seconds)
{
    SETTINGS_PROFILE("seconds: %d", seconds);
    if (this->autologin_timeout_get() != seconds)
    {
        m_prefs->set_autologin_delay(seconds);
        if (!m_prefs->save())
        {
            DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_GREETER_SYNC_TO_FILE_FAILED_3);
        }

        this->autologin_timeout_set(seconds);
    }

    invocation.ret();
}

void GreeterDBus::change_hide_user_list_authorized_cb(MethodInvocation invocation, bool hide)
{
    SETTINGS_PROFILE("hide: %d", hide);
    if (this->hide_user_list_get() != hide)
    {
        m_prefs->set_hide_user_list(hide);
        if (!m_prefs->save())
        {
            DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_GREETER_SYNC_TO_FILE_FAILED_4);
        }

        this->hide_user_list_set(hide);
    }

    invocation.ret();
}

void GreeterDBus::change_allow_manual_login_authorized_cb(MethodInvocation invocation, bool allow)
{
    SETTINGS_PROFILE("allow: %d", allow);
    if (this->allow_manual_login_get() != allow)
    {
        m_prefs->set_enable_manual_login(allow);
        if (!m_prefs->save())
        {
            DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_GREETER_SYNC_TO_FILE_FAILED_5);
        }

        this->allow_manual_login_set(allow);
    }

    invocation.ret();
}

void GreeterDBus::change_scale_mode_authorized_cb(MethodInvocation invocation, guint16 mode, guint16 factor)
{
    SETTINGS_PROFILE("mode: %d factor: %d", mode, factor);

    if (mode < GREETER_SCALING_MODE_AUTO || mode > GREETER_SCALING_MODE_LAST)
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_GREETER_SCALE_MODE_INVALIDE);
    }
    //*
    m_prefs->set_scale_mode((GreeterScalingMode)mode);
    m_prefs->set_scale_factor(factor);
    if (!m_prefs->save())
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_GREETER_SYNC_TO_FILE_FAILED_6);
    }

    this->scale_mode_set(mode);
    this->scale_factor_set(factor);

    invocation.ret();
}

bool GreeterDBus::reload_greeter_settings()
{
    SETTINGS_PROFILE("");

    m_prefs->load();
    this->background_set(m_prefs->get_background_file());
    this->autologin_user_set(m_prefs->get_autologin_user());
    this->autologin_timeout_set(m_prefs->get_autologin_delay());
    this->hide_user_list_set(m_prefs->get_hide_user_list());
    this->allow_manual_login_set(m_prefs->get_enable_manual_login());
    this->scale_mode_set(m_prefs->get_scale_mode());
    this->scale_factor_set(m_prefs->get_scale_factor());

    return true;
}

void GreeterDBus::on_bus_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name)
{
    if (!connect)
    {
        LOG_WARNING("failed to connect dbus. name: %s", name.c_str());
        return;
    }
    try
    {
        this->object_register_id_ = this->register_object(connect, GREETER_OBJECT_PATH);
    }
    catch (const Glib::Error &e)
    {
        LOG_WARNING("register object_path %s fail: %s.", GREETER_OBJECT_PATH, e.what().c_str());
    }
}

void GreeterDBus::on_name_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name)
{
    LOG_DEBUG("success to register dbus name: %s", name.c_str());
}

void GreeterDBus::on_name_lost(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name)
{
    LOG_WARNING("failed to register dbus name: %s", name.c_str());
}

void GreeterDBus::on_autologin_delay_changed()
{
    this->autologin_timeout_set(m_prefs->get_autologin_delay());
}

void GreeterDBus::on_autologin_user_changed()
{
    this->autologin_user_set(m_prefs->get_autologin_user());
}

void GreeterDBus::on_background_file_changed()
{
    this->background_set(m_prefs->get_background_file());
}

void GreeterDBus::on_enable_manual_login_changed()
{
    this->allow_manual_login_set(m_prefs->get_enable_manual_login());
}

void GreeterDBus::on_hide_user_list_changed()
{
    this->hide_user_list_set(m_prefs->get_hide_user_list());
}

void GreeterDBus::on_scale_factor_changed()
{
    this->scale_factor_set(m_prefs->get_scale_factor());
}

void GreeterDBus::on_scale_mode_changed()
{
    this->scale_mode_set(m_prefs->get_scale_mode());
}

Glib::ustring GreeterDBus::uid_to_name(uid_t uid)
{
    struct passwd *pw_ptr;
    if ((pw_ptr = getpwuid(uid)) == NULL)
    {
        LOG_WARNING("failed to find user name by uid: %d", uid);
        return Glib::ustring();
    }

    return Glib::ustring(pw_ptr->pw_name);
}

}  // namespace Kiran
