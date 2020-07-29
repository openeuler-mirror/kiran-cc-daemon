/*
 * @Author       : tangjie02
 * @Date         : 2020-06-19 13:58:22
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-07-29 17:54:22
 * @Description  : 
 * @FilePath     : /kiran-system-daemon/plugins/accounts/user.cpp
 */

#include "plugins/accounts/user.h"

#include <fmt/format.h>
#include <gio/gunixinputstream.h>
#include <glib/gstdio.h>
#include <grp.h>

#include <cinttypes>

#include "lib/auth-manager.h"
#include "lib/log.h"
#include "plugins/accounts/accounts-common.h"
#include "plugins/accounts/accounts-manager.h"
#include "plugins/accounts/accounts-util.h"
#include "plugins/accounts/user-classify.h"

namespace Kiran
{
#define ACCOUNTS_USER_OBJECT_PATH "/com/unikylin/Kiran/SystemDaemon/Accounts/User"
#define ADMIN_GROUP "wheel"

User::User(uint64_t uid) : object_register_id_(0),
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
    SETTINGS_PROFILE("Uid: %" PRIu64, this->uid_);
    this->object_path_ = fmt::format(ACCOUNTS_USER_OBJECT_PATH "/{0}", this->Uid_get());
    try
    {
        this->dbus_connect_ = Gio::DBus::Connection::get_sync(Gio::DBus::BUS_TYPE_SYSTEM);
    }
    catch (const Glib::Error &e)
    {
        LOG_WARNING("failed to get system bus: %s.", e.what().c_str());
        return;
    }

    this->object_register_id_ = this->register_object(this->dbus_connect_, this->object_path_.c_str());
}

void User::dbus_unregister()
{
    SETTINGS_PROFILE("Uid: %" PRIu64, this->uid_);

    if (this->object_register_id_)
    {
        this->unregister_object();
        this->object_register_id_ = 0;
    }
}

void User::update_from_passwd_shadow(PasswdShadow passwd_shadow)
{
    Glib::ustring real_name;

    this->freeze_notify();

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

    auto account_type = this->account_type_from_pwent(this->passwd_);
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
    this->LocalAccount_set(!is_system_account);

    this->thaw_notify();
}

void User::save_data()
{
    SETTINGS_PROFILE("UserName: %s", this->UserName_get().c_str());
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

void User::freeze_notify()
{
    SETTINGS_PROFILE("Uid: %" PRIu64, this->uid_);
    if (this->dbus_connect_)
    {
        this->dbus_connect_->freeze_notify();
    }
}

void User::thaw_notify()
{
    SETTINGS_PROFILE("Uid: %" PRIu64, this->uid_);
    if (this->dbus_connect_)
    {
        this->dbus_connect_->thaw_notify();
    }
}

#define USER_SET_ZERO_PROP_AUTH(fun, callback, auth)                                                            \
    void User::fun(MethodInvocation &invocation)                                                                \
    {                                                                                                           \
        SETTINGS_PROFILE("");                                                                                   \
        std::string action_id = this->get_auth_action(invocation, auth);                                        \
        RETURN_IF_TRUE(action_id.empty());                                                                      \
                                                                                                                \
        AuthManager::get_instance()->start_auth_check(action_id,                                                \
                                                      TRUE,                                                     \
                                                      invocation.getMessage(),                                  \
                                                      std::bind(&User::callback, this, std::placeholders::_1)); \
                                                                                                                \
        return;                                                                                                 \
    }

#define USER_SET_ONE_PROP_AUTH(fun, callback, auth, type1)                                                             \
    void User::fun(type1 value,                                                                                        \
                   MethodInvocation &invocation)                                                                       \
    {                                                                                                                  \
        SETTINGS_PROFILE("");                                                                                          \
        std::string action_id = this->get_auth_action(invocation, auth);                                               \
        RETURN_IF_TRUE(action_id.empty());                                                                             \
                                                                                                                       \
        AuthManager::get_instance()->start_auth_check(action_id,                                                       \
                                                      TRUE,                                                            \
                                                      invocation.getMessage(),                                         \
                                                      std::bind(&User::callback, this, std::placeholders::_1, value)); \
                                                                                                                       \
        return;                                                                                                        \
    }

#define USER_SET_TWO_PROP_AUTH(fun, callback, auth, type1, type2)                                                               \
    void User::fun(type1 value1,                                                                                                \
                   type2 value2,                                                                                                \
                   MethodInvocation &invocation)                                                                                \
    {                                                                                                                           \
        SETTINGS_PROFILE("");                                                                                                   \
        std::string action_id = this->get_auth_action(invocation, auth);                                                        \
        RETURN_IF_TRUE(action_id.empty());                                                                                      \
                                                                                                                                \
        AuthManager::get_instance()->start_auth_check(action_id,                                                                \
                                                      TRUE,                                                                     \
                                                      invocation.getMessage(),                                                  \
                                                      std::bind(&User::callback, this, std::placeholders::_1, value1, value2)); \
                                                                                                                                \
        return;                                                                                                                 \
    }

USER_SET_ONE_PROP_AUTH(SetUserName, change_user_name_authorized_cb, AUTH_USER_ADMIN, const Glib::ustring &);
USER_SET_ONE_PROP_AUTH(SetRealName, change_real_name_authorized_cb, AUTH_CHANGE_OWN_USER_DATA, const Glib::ustring &);
USER_SET_ONE_PROP_AUTH(SetEmail, change_email_authorized_cb, AUTH_CHANGE_OWN_USER_DATA, const Glib::ustring &);
USER_SET_ONE_PROP_AUTH(SetLanguage, change_language_authorized_cb, AUTH_CHANGE_OWN_USER_DATA, const Glib::ustring &);
USER_SET_ONE_PROP_AUTH(SetXSession, change_x_session_authorized_cb, AUTH_CHANGE_OWN_USER_DATA, const Glib::ustring &);
USER_SET_ONE_PROP_AUTH(SetSession, change_session_authorized_cb, AUTH_CHANGE_OWN_USER_DATA, const Glib::ustring &);
USER_SET_ONE_PROP_AUTH(SetSessionType, change_session_type_authorized_cb, AUTH_CHANGE_OWN_USER_DATA, const Glib::ustring &);
USER_SET_ONE_PROP_AUTH(SetLocation, change_location_authorized_cb, AUTH_CHANGE_OWN_USER_DATA, const Glib::ustring &);
USER_SET_ONE_PROP_AUTH(SetHomeDirectory, change_home_dir_authorized_cb, AUTH_USER_ADMIN, const Glib::ustring &);
USER_SET_ONE_PROP_AUTH(SetShell, change_shell_authorized_cb, AUTH_USER_ADMIN, const Glib::ustring &);
USER_SET_ONE_PROP_AUTH(SetIconFile, change_icon_file_authorized_cb, AUTH_CHANGE_OWN_USER_DATA, const Glib::ustring &);
USER_SET_ONE_PROP_AUTH(SetLocked, change_locked_authorized_cb, AUTH_USER_ADMIN, bool);
USER_SET_ONE_PROP_AUTH(SetAccountType, change_account_type_authorized_cb, AUTH_CHANGE_OWN_USER_DATA, int32_t);
USER_SET_ONE_PROP_AUTH(SetPasswordMode, change_password_mode_authorized_cb, AUTH_CHANGE_OWN_USER_DATA, int32_t);
USER_SET_TWO_PROP_AUTH(SetPassword, change_password_authorized_cb, AUTH_CHANGE_OWN_PASSWORD, const Glib::ustring &, const Glib::ustring &);
USER_SET_ONE_PROP_AUTH(SetPasswordHint, change_password_hint_authorized_cb, AUTH_CHANGE_OWN_USER_DATA, const Glib::ustring &);
USER_SET_ONE_PROP_AUTH(SetAutomaticLogin, change_auto_login_authorized_cb, AUTH_USER_ADMIN, bool);
USER_SET_ZERO_PROP_AUTH(GetPasswordExpirationPolicy, get_password_expiration_policy_authorized_cb, AUTH_CHANGE_OWN_USER_DATA);

std::string User::get_auth_action(MethodInvocation &invocation, const std::string &own_action)
{
    RETURN_VAL_IF_TRUE(own_action == AUTH_USER_ADMIN, AUTH_USER_ADMIN);

    std::string action_id;
    int32_t uid;

    if (!AccountsUtil::get_caller_uid(invocation.getMessage(), uid))
    {
        invocation.ret(Glib::Error(ACCOUNTS_ERROR, int32_t(AccountsError::ERROR_FAILED), "identifying caller failed"));
        return std::string();
    }

    if (this->Uid_get() == (uid_t)uid)
    {
        return own_action;
    }
    else
    {
        return AUTH_USER_ADMIN;
    }
}

void User::change_user_name_authorized_cb(MethodInvocation invocation, const Glib::ustring &name)
{
    SETTINGS_PROFILE("UserName: %s", name.c_str());

    if (this->UserName_get() != name)
    {
        auto old_name = this->UserName_get();

        SPAWN_WITH_LOGIN_UID(invocation, "/usr/sbin/usermod", "-l", name, "--", this->UserName_get().raw());

        this->UserName_set(name);
        this->move_extra_data(old_name, name);
    }

    invocation.ret();
}

void User::change_real_name_authorized_cb(MethodInvocation invocation, const Glib::ustring &name)
{
    SETTINGS_PROFILE("RealName: %s", name.c_str());
    if (this->RealName_get() != name)
    {
        SPAWN_WITH_LOGIN_UID(invocation, "/usr/sbin/usermod", "-c", name, "--", this->UserName_get().raw());

        this->RealName_set(name);
    }

    invocation.ret();
}

#define USER_AUTH_CHECK_CB(fun, prop)                                       \
    void User::fun(MethodInvocation invocation, const Glib::ustring &value) \
    {                                                                       \
        SETTINGS_PROFILE(#prop ": %s", value.c_str());                      \
        if (this->prop##_get() != value)                                    \
        {                                                                   \
            this->prop##_set(value);                                        \
            this->save_data();                                              \
        }                                                                   \
                                                                            \
        invocation.ret();                                                   \
    }

USER_AUTH_CHECK_CB(change_email_authorized_cb, Email);
USER_AUTH_CHECK_CB(change_language_authorized_cb, Language);
USER_AUTH_CHECK_CB(change_x_session_authorized_cb, XSession);
USER_AUTH_CHECK_CB(change_session_authorized_cb, Session);
USER_AUTH_CHECK_CB(change_session_type_authorized_cb, SessionType);
USER_AUTH_CHECK_CB(change_location_authorized_cb, Location);

void User::change_home_dir_authorized_cb(MethodInvocation invocation, const Glib::ustring &home_dir)
{
    SETTINGS_PROFILE("HomeDir: %s", home_dir.c_str());

    if (this->HomeDirectory_get() != home_dir)
    {
        SPAWN_WITH_LOGIN_UID(invocation, "/usr/sbin/usermod", "-m", "-d", home_dir, "--", this->UserName_get().raw())

        this->HomeDirectory_set(home_dir);
        this->reset_icon_file();
    }
    invocation.ret();
}

void User::change_shell_authorized_cb(MethodInvocation invocation, const Glib::ustring &shell)
{
    SETTINGS_PROFILE("Shell: %s", shell.c_str());

    if (this->Shell_get() != shell)
    {
        SPAWN_WITH_LOGIN_UID(invocation, "/usr/sbin/usermod", "-s", shell, "--", this->UserName_get().raw());
        this->Shell_set(shell);
    }
    invocation.ret();
}

void User::become_user(std::shared_ptr<Passwd> passwd)
{
}

void User::change_icon_file_authorized_cb(MethodInvocation invocation, const Glib::ustring &icon_file)
{
    SETTINGS_PROFILE("IconFile: %s", icon_file.c_str());

    auto filename = icon_file;

    do
    {
        if (icon_file.empty())
        {
            auto path = Glib::build_filename(ICONDIR, this->UserName_get());
            g_remove(path.c_str());
            break;
        }

        auto file = Gio::File::create_for_path(icon_file);
        filename = file->get_path();
        Glib::RefPtr<Gio::FileInfo> file_info;
        try
        {
            file_info = file->query_info(G_FILE_ATTRIBUTE_UNIX_MODE "," G_FILE_ATTRIBUTE_STANDARD_TYPE "," G_FILE_ATTRIBUTE_STANDARD_SIZE);
        }
        catch (const Glib::Error &e)
        {
            invocation.ret(Glib::Error(ACCOUNTS_ERROR, int32_t(AccountsError::ERROR_FAILED), e.what().c_str()));
            return;
        }

        auto size = file_info->get_attribute_uint64(G_FILE_ATTRIBUTE_STANDARD_SIZE);

        if (file_info->get_file_type() != Gio::FileType::FILE_TYPE_REGULAR)
        {
            auto err_message = fmt::format("file '{0}' is not a regular file", filename.raw());
            invocation.ret(Glib::Error(ACCOUNTS_ERROR, int32_t(AccountsError::ERROR_FAILED), err_message.c_str()));
            return;
        }

        if (size > 1048576)
        {
            auto err_message = fmt::format("file '%s' is too large to be used as an icon", filename.raw());
            invocation.ret(Glib::Error(ACCOUNTS_ERROR, int32_t(AccountsError::ERROR_FAILED), err_message.c_str()));
            return;
        }

        auto mode = file_info->get_attribute_uint64(G_FILE_ATTRIBUTE_UNIX_MODE);
        if ((mode & S_IROTH) != 0 && Glib::str_has_prefix(filename, ICONDIR))
        {
            break;
        }

        // copy file to directory ICONDIR
        int32_t uid;
        if (!AccountsUtil::get_caller_uid(invocation.getMessage(), uid))
        {
            invocation.ret(Glib::Error(ACCOUNTS_ERROR, int32_t(AccountsError::ERROR_FAILED), "failed to copy file, could not determine caller UID"));
            return;
        }

        auto dest_path = Glib::build_filename(ICONDIR, this->UserName_get());
        auto dest_file = Gio::File::create_for_path(dest_path);
        Glib::RefPtr<Gio::FileOutputStream> output;
        try
        {
            output = dest_file->replace();
        }
        catch (const Glib::Error &e)
        {
            auto err_message = fmt::format("creating file '{0}' failed: {1}", dest_path, e.what().raw());
            invocation.ret(Glib::Error(ACCOUNTS_ERROR, int32_t(AccountsError::ERROR_FAILED), err_message.c_str()));
            return;
        }

        std::vector<std::string> argv = {"/bin/cat", filename.raw()};
        auto pwent = AccountsWrapper::get_instance()->get_passwd_by_uid(uid);

        int32_t std_out;
        try
        {
            Glib::spawn_async_with_pipes(std::string(),
                                         argv,
                                         Glib::SPAWN_DEFAULT,
                                         sigc::bind(sigc::mem_fun(this, &User::become_user), pwent),
                                         nullptr,
                                         nullptr,
                                         &std_out,
                                         nullptr);
        }
        catch (const Glib::Error &e)
        {
            auto err_message = fmt::format("reading file '{0}' failed: {1}", filename.raw(), e.what().raw());
            invocation.ret(Glib::Error(ACCOUNTS_ERROR, int32_t(AccountsError::ERROR_FAILED), err_message.c_str()));
            return;
        }

        auto input = Glib::wrap(g_unix_input_stream_new(std_out, false));
        gssize bytes = 0;
        std::string err;
        try
        {
            bytes = output->splice(input, Gio::OUTPUT_STREAM_SPLICE_CLOSE_TARGET);
        }
        catch (const Glib::Error &e)
        {
            err = e.what().raw();
        }

        if (bytes < 0 || (uint64_t)bytes != size)
        {
            auto err_message = fmt::format("copying file '{0}' to '{1}' failed: {2}", filename.raw(), dest_path, err.c_str());
            invocation.ret(Glib::Error(ACCOUNTS_ERROR, int32_t(AccountsError::ERROR_FAILED), err_message.c_str()));
            IGNORE_EXCEPTION(dest_file->remove());
            return;
        }
        filename = dest_path;
    } while (0);

    this->IconFile_set(filename);
    this->save_data();
}

void User::change_locked_authorized_cb(MethodInvocation invocation, bool locked)
{
    SETTINGS_PROFILE("Locked: %d", locked);

    if (this->Locked_get() != locked)
    {
        SPAWN_WITH_LOGIN_UID(invocation, "/usr/sbin/usermod", locked ? "-L" : "-U", "--", this->UserName_get().raw());
        this->Locked_set(locked);
        if (this->AutomaticLogin_get() && locked)
        {
            std::string err;
            if (!AccountsManager::get_instance()->set_automatic_login(this->shared_from_this(), false, err))
            {
                LOG_WARNING("%s", err.c_str());
            }
            this->AutomaticLogin_set(false);
        }
    }
    invocation.ret();
}

void User::change_account_type_authorized_cb(MethodInvocation invocation, int32_t account_type)
{
    SETTINGS_PROFILE("AccountType: %d", account_type);

    if (this->AccountType_get() != account_type)
    {
        auto grp = AccountsWrapper::get_instance()->get_group_by_name(ADMIN_GROUP);
        if (!grp)
        {
            invocation.ret(Glib::Error(ACCOUNTS_ERROR, int32_t(AccountsError::ERROR_FAILED), "failed to set account type: " ADMIN_GROUP " group not found"));
            return;
        }
        auto admin_gid = grp->gr_gid;

        auto groups = AccountsWrapper::get_instance()->get_user_groups(this->UserName_get(), this->get_gid());
        std::string groups_join;
        for (auto i = 0; i < (int)groups.size(); ++i)
        {
            if (groups[i] != admin_gid)
            {
                groups_join += fmt::format("{0}{1}", groups_join.empty() ? std::string() : std::string(","), groups[i]);
            }
        }

        if (account_type == int32_t(AccountType::ACCOUNT_TYPE_ADMINISTRATOR))
        {
            groups_join += fmt::format("{0}{1}", groups_join.empty() ? std::string() : std::string(","), admin_gid);
        }

        SPAWN_WITH_LOGIN_UID(invocation, "/usr/sbin/usermod", "-G", groups_join, "--", this->UserName_get().raw());
        this->AccountType_set(account_type);
    }
    invocation.ret();
}

void User::change_password_mode_authorized_cb(MethodInvocation invocation, int32_t password_mode)
{
    SETTINGS_PROFILE("PasswordMode: %d", password_mode);

    if (this->PasswordMode_get() != password_mode)
    {
        this->freeze_notify();

        if (password_mode == int32_t(PasswordMode::PASSWORD_MODE_SET_AT_LOGIN) ||
            password_mode == int32_t(PasswordMode::PASSWORD_MODE_NONE))
        {
            SPAWN_WITH_LOGIN_UID(invocation, "/usr/bin/passwd", "-d", "--", this->UserName_get().raw());

            if (password_mode == int32_t(PasswordMode::PASSWORD_MODE_SET_AT_LOGIN))
            {
                SPAWN_WITH_LOGIN_UID(invocation, "/usr/bin/chage", "-d", "0", "--", this->UserName_get().raw());
            }

            this->PasswordHint_set(std::string());
        }
        else if (this->Locked_get())
        {
            SPAWN_WITH_LOGIN_UID(invocation, "/usr/sbin/usermod", "-U", "--", this->UserName_get().raw());
        }
        this->Locked_set(false);
        this->PasswordMode_set(password_mode);
        this->save_data();
        this->thaw_notify();
    }

    invocation.ret();
}

void User::change_password_authorized_cb(MethodInvocation invocation, const Glib::ustring &password, const Glib::ustring &password_hint)
{
    SETTINGS_PROFILE("Password: %s PasswordHint: %s", password.c_str(), password_hint.c_str());

    this->freeze_notify();

    std::vector<std::string> argv = {"/usr/sbin/usermod", "-p", password.raw(), "--", this->UserName_get().raw()};
    std::string err;
    if (!AccountsUtil::spawn_with_login_uid(invocation.getMessage(), argv, err))
    {
        err = fmt::format("running '{0}' failed: {1}", argv[0], err);
        invocation.ret(Glib::Error(ACCOUNTS_ERROR, int32_t(AccountsError::ERROR_FAILED), err.c_str()));
        return;
    }

    this->PasswordMode_set(int32_t(PasswordMode::PASSWORD_MODE_REGULAR));
    this->Locked_set(false);
    this->PasswordHint_set(password_hint);
    this->save_data();

    this->thaw_notify();
    invocation.ret();
}

USER_AUTH_CHECK_CB(change_password_hint_authorized_cb, PasswordHint);

void User::change_auto_login_authorized_cb(MethodInvocation invocation, bool auto_login)
{
    SETTINGS_PROFILE("AutoLogin: %d", auto_login);

    if (this->Locked_get())
    {
        invocation.ret(Glib::Error(ACCOUNTS_ERROR, int32_t(AccountsError::ERROR_FAILED), "failed to change automatic login: user is locked"));
        return;
    }
    std::string err;
    if (!AccountsManager::get_instance()->set_automatic_login(this->shared_from_this(), auto_login, err))
    {
        auto err_message = fmt::format("failed to change automatic login: {0}", err);
        invocation.ret(Glib::Error(ACCOUNTS_ERROR, int32_t(AccountsError::ERROR_FAILED), err_message.c_str()));
        return;
    }
    invocation.ret();
}

void User::get_password_expiration_policy_authorized_cb(MethodInvocation invocation)
{
    SETTINGS_PROFILE("");
    if (!this->spwd_)
    {
        invocation.ret(Glib::Error(ACCOUNTS_ERROR, int32_t(AccountsError::ERROR_FAILED), "account expiration policy unknown to accounts service"));
        return;
    }

    invocation.ret(this->spwd_->sp_expire,
                   this->spwd_->sp_lstchg,
                   this->spwd_->sp_min,
                   this->spwd_->sp_max,
                   this->spwd_->sp_warn,
                   this->spwd_->sp_inact);
}

AccountType User::account_type_from_pwent(std::shared_ptr<Passwd> passwd)
{
    g_return_val_if_fail(passwd, AccountType::ACCOUNT_TYPE_STANDARD);

    struct group *grp;
    gint i;

    if (passwd->pw_uid == 0)
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
        if (g_strcmp0(grp->gr_mem[i], passwd->pw_name.c_str()) == 0)
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
    this->freeze_notify();

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

    this->thaw_notify();
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
    if (keyfile->has_group("User"))
    {
        IGNORE_EXCEPTION(keyfile->remove_group("User"));
    }

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

}  // namespace Kiran