/*
 * @Author       : tangjie02
 * @Date         : 2020-06-19 13:58:22
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-07-27 16:52:01
 * @Description  : 
 * @FilePath     : /kiran-system-daemon/plugins/accounts/user.cpp
 */

#include "plugins/accounts/user.h"

#include <fmt/format.h>
#include <glib/gstdio.h>
#include <grp.h>

#include "lib/auth-manager.h"
#include "lib/log.h"
#include "plugins/accounts/accounts-common.h"
#include "plugins/accounts/accounts-util.h"
#include "plugins/accounts/user-classify.h"

namespace Kiran
{
#define ACCOUNTS_USER_OBJECT_PATH "/com/unikylin/Kiran/SystemDaemon/Accounts/User"
#define ADMIN_GROUP "wheel"

User::User(uint64_t uid) : dbus_connect_id_(0),
                           object_register_id_(0),
                           uid_(uid)

{
    this->keyfile_ = std::make_shared<Glib::KeyFile>();
}

User::~User()
{
    this->dbus_unregister();
}

void User::dbus_register()
{
    this->object_path_ = fmt::format(ACCOUNTS_USER_OBJECT_PATH "/{0}", this->Uid_get());
    this->dbus_connect_id_ = Gio::DBus::own_name(Gio::DBus::BUS_TYPE_SYSTEM,
                                                 ACCOUNTS_DBUS_NAME,
                                                 sigc::mem_fun(this, &User::on_bus_acquired),
                                                 sigc::mem_fun(this, &User::on_name_acquired),
                                                 sigc::mem_fun(this, &User::on_name_lost));
}

void User::dbus_unregister()
{
    if (this->dbus_connect_id_)
    {
        Gio::DBus::unown_name(this->dbus_connect_id_);
        this->dbus_connect_id_ = 0;
    }

    if (this->object_register_id_)
    {
        this->unregister_object();
        this->object_register_id_ = 0;
    }
}

void User::update_from_passwd_shadow(PasswdShadow passwd_shadow)
{
    Glib::ustring real_name;

    this->dbus_connect_->freeze_notify();

    this->passwd_ = passwd_shadow.first;
    this->spwd_ = passwd_shadow.second;

    if (!this->passwd_->pw_gecos.empty())
    {
        if (Glib::ustring(this->passwd_->pw_gecos).validate())
        {
            real_name = this->passwd_->pw_gecos;
        }
        else
        {
            LOG_WARNING("User %s has invalid UTF-8 in GECOS field.  It would be a good thing to check /etc/passwd.",
                        this->passwd_->pw_name.c_str() ? this->passwd_->pw_name.c_str() : "");
        }
    }

    this->RealName_set(real_name);
    this->Uid_set(this->passwd_->pw_uid);

    auto account_type = this->account_type_from_pwent(*this->passwd_);
    this->AccountType_set(int32_t(account_type));

    this->UserName_set(this->passwd_->pw_name);
    this->HomeDirectory_set(this->passwd_->pw_dir);
    this->Shell_set(this->passwd_->pw_shell);
    this->reset_icon_file();

    std::shared_ptr<std::string> passwd;
    bool locked = false;

    if (this->spwd_)
        passwd = this->spwd_->sp_pwdp;

    if (passwd && passwd->length() > 0 && passwd->at(0) == '!')
    {
        locked = true;
    }
    else
    {
        locked = false;
    }

    this->Locked_set(locked);

    PasswordMode mode;

    if (!passwd || !passwd->empty())
    {
        mode = PasswordMode::PASSWORD_MODE_REGULAR;
    }
    else
    {
        mode = PasswordMode::PASSWORD_MODE_NONE;
    }

    if (this->spwd_ && this->spwd_->sp_lstchg == 0)
    {
        mode = PasswordMode::PASSWORD_MODE_SET_AT_LOGIN;
    }

    this->PasswordMode_set(int32_t(mode));

    auto is_system_account = !UserClassify::is_human(this->passwd_->pw_uid, this->passwd_->pw_name, this->passwd_->pw_shell);
    this->SystemAccount_set(is_system_account);

    this->dbus_connect_->thaw_notify();
}

void User::save_data()
{
    this->save_to_keyfile(this->keyfile_);
    try
    {
        auto data = this->keyfile_->to_data();
        auto filename = Glib::build_filename(USERDIR, this->UserName_get());
        Glib::file_set_contents(filename, data);

        this->Saved_set(true);
    }
    catch (const Glib::Error &e)
    {
        LOG_WARNING("Saving data for user %s failed: %s", this->UserName_get().c_str(), e.what().c_str());
    }
}

#define USER_START_AUTH_CHECK(fun, callback, type)                                                                     \
    void User::fun(type value,                                                                                         \
                   MethodInvocation &invocation)                                                                       \
    {                                                                                                                  \
        std::string action_id = this->get_change_user_data_action(invocation);                                         \
        RETURN_IF_TRUE(action_id.empty());                                                                             \
                                                                                                                       \
        AuthManager::get_instance()->start_auth_check(action_id,                                                       \
                                                      TRUE,                                                            \
                                                      invocation.getMessage(),                                         \
                                                      std::bind(&User::callback, this, std::placeholders::_1, value)); \
                                                                                                                       \
        return;                                                                                                        \
    }

#define USER_START_AUTH_CHECK_PASSWORD(fun, callback, type1, type2)                                                             \
    void User::fun(type1 value1,                                                                                                \
                   type2 value2,                                                                                                \
                   MethodInvocation &invocation)                                                                                \
    {                                                                                                                           \
        std::string action_id = this->get_change_password_action(invocation);                                                   \
        RETURN_IF_TRUE(action_id.empty());                                                                                      \
                                                                                                                                \
        AuthManager::get_instance()->start_auth_check(action_id,                                                                \
                                                      TRUE,                                                                     \
                                                      invocation.getMessage(),                                                  \
                                                      std::bind(&User::callback, this, std::placeholders::_1, value1, value2)); \
                                                                                                                                \
        return;                                                                                                                 \
    }

#define USER_START_AUTH_CHECK_ADMIN(fun, callback, type)                                                               \
    void User::fun(type value,                                                                                         \
                   MethodInvocation &invocation)                                                                       \
    {                                                                                                                  \
        AuthManager::get_instance()->start_auth_check(AUTH_USER_ADMIN,                                                 \
                                                      TRUE,                                                            \
                                                      invocation.getMessage(),                                         \
                                                      std::bind(&User::callback, this, std::placeholders::_1, value)); \
                                                                                                                       \
        return;                                                                                                        \
    }

USER_START_AUTH_CHECK_ADMIN(SetUserName, change_user_name_authorized_cb, const Glib::ustring &);
USER_START_AUTH_CHECK(SetRealName, change_real_name_authorized_cb, const Glib::ustring &);
USER_START_AUTH_CHECK(SetEmail, change_email_authorized_cb, const Glib::ustring &);
USER_START_AUTH_CHECK(SetLanguage, change_language_authorized_cb, const Glib::ustring &);
USER_START_AUTH_CHECK(SetXSession, change_x_session_authorized_cb, const Glib::ustring &);
USER_START_AUTH_CHECK(SetSession, change_session_authorized_cb, const Glib::ustring &);
USER_START_AUTH_CHECK(SetSessionType, change_session_type_authorized_cb, const Glib::ustring &);
USER_START_AUTH_CHECK(SetLocation, change_location_authorized_cb, const Glib::ustring &);
USER_START_AUTH_CHECK_ADMIN(SetHomeDirectory, change_home_dir_authorized_cb, const Glib::ustring &);
USER_START_AUTH_CHECK_ADMIN(SetShell, change_shell_authorized_cb, const Glib::ustring &);
USER_START_AUTH_CHECK(SetIconFile, change_icon_file_authorized_cb, const Glib::ustring &);
USER_START_AUTH_CHECK_ADMIN(SetLocked, change_locked_authorized_cb, bool);
USER_START_AUTH_CHECK(SetAccountType, change_account_type_authorized_cb, int32_t);
USER_START_AUTH_CHECK(SetPasswordMode, change_password_mode_authorized_cb, int32_t);
USER_START_AUTH_CHECK_PASSWORD(SetPassword, change_password_authorized_cb, const Glib::ustring &, const Glib::ustring &);
USER_START_AUTH_CHECK(SetPasswordHint, change_password_hint_authorized_cb, const Glib::ustring &);
USER_START_AUTH_CHECK_ADMIN(SetAutomaticLogin, change_auto_login_authorized_cb, bool);

std::string User::get_change_user_data_action(MethodInvocation &invocation)
{
    std::string action_id;
    int32_t uid;

    if (!AccountsUtil::get_caller_uid(invocation.getMessage(), uid))
    {
        invocation.ret(Glib::Error(ACCOUNTS_ERROR, static_cast<int32_t>(AccountsError::ERROR_FAILED), "identifying caller failed"));
        return std::string();
    }

    if (this->Uid_get() == (uid_t)uid)
    {
        return AUTH_CHANGE_OWN_USER_DATA;
    }
    else
    {
        return AUTH_USER_ADMIN;
    }
}

std::string User::get_change_password_action(MethodInvocation &invocation)
{
    std::string action_id;
    int32_t uid;

    if (!AccountsUtil::get_caller_uid(invocation.getMessage(), uid))
    {
        invocation.ret(Glib::Error(ACCOUNTS_ERROR, static_cast<int32_t>(AccountsError::ERROR_FAILED), "identifying caller failed"));
        return std::string();
    }

    if (this->Uid_get() == (uid_t)uid)
    {
        return AUTH_CHANGE_OWN_PASSWORD;
    }
    else
    {
        return AUTH_USER_ADMIN;
    }
}

void User::change_user_name_authorized_cb(const Glib::RefPtr<Gio::DBus::MethodInvocation> invocation, const Glib::ustring &name)
{
    if (this->UserName_get() != name)
    {
        auto old_name = this->UserName_get();
        LOG_DEBUG("change name of user '%s' (%d) to '%s'",
                  old_name.c_str(),
                  (int32_t)this->Uid_get(),
                  name.c_str());

        std::vector<std::string> argv = {"/usr/sbin/usermod", "-l", name, "--", this->UserName_get().raw()};
        std::string err;

        if (!AccountsUtil::spawn_with_login_uid(invocation, argv, err))
        {
            err = fmt::format("running '{0}' failed: {1}", argv[0], err);
            invocation->return_error(Glib::Error(ACCOUNTS_ERROR, static_cast<int32_t>(AccountsError::ERROR_FAILED), err.c_str()));
            return;
        }

        this->UserName_set(name);
        this->move_extra_data(old_name, name);
    }

    INVOCATION_RETURN(invocation);
}

void User::change_real_name_authorized_cb(const Glib::RefPtr<Gio::DBus::MethodInvocation> invocation, const Glib::ustring &name)
{
    if (this->RealName_get() != name)
    {
        LOG_DEBUG("change real name of user '%s' (%d) to '%s'",
                  this->UserName_get().c_str(),
                  (int32_t)this->Uid_get(),
                  name.c_str());

        std::vector<std::string> argv = {"/usr/sbin/usermod", "-c", name, "--", this->UserName_get().raw()};
        std::string err;

        if (!AccountsUtil::spawn_with_login_uid(invocation, argv, err))
        {
            err = fmt::format("running '{0}' failed: {1}", argv[0], err);
            invocation->return_error(Glib::Error(ACCOUNTS_ERROR, static_cast<int32_t>(AccountsError::ERROR_FAILED), err.c_str()));
            return;
        }

        this->RealName_set(name);
    }

    INVOCATION_RETURN(invocation);
}

#define USER_START_AUTH_CHECK_CB(fun, prop)                                                                \
    void User::fun(const Glib::RefPtr<Gio::DBus::MethodInvocation> invocation, const Glib::ustring &value) \
    {                                                                                                      \
        if (this->prop##_get() != value)                                                                   \
        {                                                                                                  \
            this->prop##_set(value);                                                                       \
            this->save_data();                                                                             \
        }                                                                                                  \
                                                                                                           \
        INVOCATION_RETURN(invocation);                                                                     \
    }

USER_START_AUTH_CHECK_CB(change_email_authorized_cb, Email);
USER_START_AUTH_CHECK_CB(change_language_authorized_cb, Language);
USER_START_AUTH_CHECK_CB(change_x_session_authorized_cb, XSession);
USER_START_AUTH_CHECK_CB(change_session_authorized_cb, Session);
USER_START_AUTH_CHECK_CB(change_session_type_authorized_cb, SessionType);
USER_START_AUTH_CHECK_CB(change_location_authorized_cb, Location);

void User::change_home_dir_authorized_cb(const Glib::RefPtr<Gio::DBus::MethodInvocation> invocation, const Glib::ustring &home_dir)
{
    if (this->HomeDirectory_get() != home_dir)
    {
        LOG_DEBUG("change home directory of user '%s' (%d) to '%s'",
                  this->UserName_get().c_str(),
                  (int32_t)this->Uid_get(),
                  home_dir.c_str());

        std::vector<std::string> argv = {"/usr/sbin/usermod", "-m", "-d", home_dir, "--", this->UserName_get().raw()};
        std::string err;

        if (!AccountsUtil::spawn_with_login_uid(invocation, argv, err))
        {
            err = fmt::format("running '{0}' failed: {1}", argv[0], err);
            invocation->return_error(Glib::Error(ACCOUNTS_ERROR, static_cast<int32_t>(AccountsError::ERROR_FAILED), err.c_str()));
            return;
        }

        this->HomeDirectory_set(home_dir);
        this->reset_icon_file();
    }
    INVOCATION_RETURN(invocation);
}

void User::change_shell_authorized_cb(const Glib::RefPtr<Gio::DBus::MethodInvocation> invocation, const Glib::ustring &shell)
{
    if (this->Shell_get() != shell)
    {
        LOG_DEBUG("change shell of user '%s' (%d) to '%s'",
                  this->UserName_get().c_str(),
                  (int32_t)this->Uid_get(),
                  shell.c_str());

        std::vector<std::string> argv = {"/usr/sbin/usermod", "-s", shell, "--", this->UserName_get().raw()};
        std::string err;

        if (!AccountsUtil::spawn_with_login_uid(invocation, argv, err))
        {
            err = fmt::format("running '{0}' failed: {1}", argv[0], err);
            invocation->return_error(Glib::Error(ACCOUNTS_ERROR, static_cast<int32_t>(AccountsError::ERROR_FAILED), err.c_str()));
            return;
        }

        this->Shell_set(shell);
    }
    INVOCATION_RETURN(invocation);
}

void User::change_icon_file_authorized_cb(const Glib::RefPtr<Gio::DBus::MethodInvocation> invocation, const Glib::ustring &icon_file)
{
}

void User::change_locked_authorized_cb(const Glib::RefPtr<Gio::DBus::MethodInvocation> invocation, bool locked)
{
}

void User::change_account_type_authorized_cb(const Glib::RefPtr<Gio::DBus::MethodInvocation> invocation, int32_t account_type)
{
}

void User::change_password_mode_authorized_cb(const Glib::RefPtr<Gio::DBus::MethodInvocation> invocation, int32_t password_mode)
{
}

void User::change_password_authorized_cb(const Glib::RefPtr<Gio::DBus::MethodInvocation> invocation, const Glib::ustring &password, const Glib::ustring &password_hint)
{
}

void User::change_password_hint_authorized_cb(const Glib::RefPtr<Gio::DBus::MethodInvocation> invocation, const Glib::ustring &password_hint)
{
}

void User::change_auto_login_authorized_cb(const Glib::RefPtr<Gio::DBus::MethodInvocation> invocation, bool auto_login)
{
}

AccountType User::account_type_from_pwent(Passwd pwent)
{
    struct group *grp;
    gint i;

    if (pwent.pw_uid == 0)
    {
        LOG_DEBUG("user is root so account type is administrator");
        return AccountType::ACCOUNT_TYPE_ADMINISTRATOR;
    }

    grp = getgrnam(ADMIN_GROUP);
    if (grp == NULL)
    {
        LOG_DEBUG(ADMIN_GROUP " group not found");
        return AccountType::ACCOUNT_TYPE_STANDARD;
    }

    for (i = 0; grp->gr_mem[i] != NULL; i++)
    {
        if (g_strcmp0(grp->gr_mem[i], pwent.pw_name.c_str()) == 0)
        {
            return AccountType::ACCOUNT_TYPE_ADMINISTRATOR;
        }
    }

    return AccountType::ACCOUNT_TYPE_STANDARD;
}

void User::reset_icon_file()
{
    auto icon_file = this->IconFile_get();
    auto home_dir = this->HomeDirectory_get();

    bool icon_is_default = (icon_file == this->default_icon_file_);

    this->default_icon_file_ = Glib::build_filename(home_dir, ".face");

    if (icon_is_default)
    {
        this->IconFile_set(this->default_icon_file_);
    }
}

#define SET_STR_VALUE_FROM_KEYFILE(key, fun)           \
    {                                                  \
        try                                            \
        {                                              \
            auto s = keyfile->get_string("User", key); \
            this->fun(s);                              \
        }                                              \
        catch (const Glib::KeyFileError &e)            \
        {                                              \
            LOG_DEBUG("%s", e.what().c_str());         \
        }                                              \
    }

#define SET_BOOL_VALUE_FROM_KEYFILE(key, fun)           \
    {                                                   \
        try                                             \
        {                                               \
            auto b = keyfile->get_boolean("User", key); \
            this->fun(b);                               \
        }                                               \
        catch (const Glib::KeyFileError &e)             \
        {                                               \
            LOG_DEBUG("%s", e.what().c_str());          \
        }                                               \
    }

void User::update_from_keyfile(std::shared_ptr<Glib::KeyFile> keyfile)
{
    this->dbus_connect_->freeze_notify();

    SET_STR_VALUE_FROM_KEYFILE("Language", Language_set);
    SET_STR_VALUE_FROM_KEYFILE("XSession", XSession_set);
    SET_STR_VALUE_FROM_KEYFILE("XSession", Session_set);
    SET_STR_VALUE_FROM_KEYFILE("Session", Session_set);
    SET_STR_VALUE_FROM_KEYFILE("SessionType", SessionType_set);
    SET_STR_VALUE_FROM_KEYFILE("Email", Email_set);
    SET_STR_VALUE_FROM_KEYFILE("Location", Location_set);
    SET_STR_VALUE_FROM_KEYFILE("PasswordHint", PasswordHint_set);
    SET_STR_VALUE_FROM_KEYFILE("Icon", IconFile_set);
    SET_BOOL_VALUE_FROM_KEYFILE("SystemAccount", SystemAccount_set);

    this->keyfile_ = keyfile;
    // user_set_cached(user, TRUE);
    // user_set_saved(user, TRUE);

    this->dbus_connect_->thaw_notify();
}

#define SET_STR_VALUE_TO_KEYFILE(key, fun)       \
    {                                            \
        auto s = this->fun();                    \
        if (!s.empty())                          \
        {                                        \
            keyfile->set_string("User", key, s); \
        }                                        \
    }

#define SET_BOOL_VALUE_TO_KEYFILE(key, fun)       \
    {                                             \
        auto b = this->fun();                     \
        if (!s.empty())                           \
        {                                         \
            keyfile->set_boolean("User", key, b); \
        }                                         \
    }

void User::save_to_keyfile(std::shared_ptr<Glib::KeyFile> keyfile)
{
    keyfile->remove_group("User");

    SET_STR_VALUE_TO_KEYFILE("Email", Email_get);
    SET_STR_VALUE_TO_KEYFILE("Language", Language_get);
    SET_STR_VALUE_TO_KEYFILE("Session", Session_get);
    SET_STR_VALUE_TO_KEYFILE("SessionType", SessionType_get);
    SET_STR_VALUE_TO_KEYFILE("XSession", XSession_get);
    SET_STR_VALUE_TO_KEYFILE("Location", Location_get);
    SET_STR_VALUE_TO_KEYFILE("PasswordHint", PasswordHint_get);
    SET_STR_VALUE_TO_KEYFILE("Icon", IconFile_get);

    keyfile->set_boolean("User", "SystemAccount", this->SystemAccount_get());
}

void User::move_extra_data(const std::string &old_name, const std::string &new_name)
{
    auto old_filename = Glib::build_filename(USERDIR, old_name);
    auto new_filename = Glib::build_filename(USERDIR, new_name);
    g_rename(old_filename.c_str(), new_filename.c_str());
}

void User::update_local_account_property(bool local)
{
    this->LocalAccount_set(local);
}

void User::update_system_account_property(bool system)
{
    this->SystemAccount_set(system);
}

void User::on_bus_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name)
{
    try
    {
        this->dbus_connect_ = connect;
        this->object_register_id_ = this->register_object(connect, this->object_path_.c_str());
    }
    catch (const Glib::Error &e)
    {
        LOG_WARNING("register object_path %s fail: %s.", this->object_path_.c_str(), e.what().c_str());
    }
}

void User::on_name_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name)
{
    LOG_DEBUG("success to register dbus name: %s", name.c_str());
}

void User::on_name_lost(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name)
{
    LOG_WARNING("failed to register dbus name: %s", name.c_str());
}
}  // namespace Kiran