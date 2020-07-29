/*
 * @Author       : tangjie02
 * @Date         : 2020-06-19 10:09:05
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-07-29 08:55:01
 * @Description  : 
 * @FilePath     : /kiran-system-daemon/plugins/accounts/accounts-manager.cpp
 */

#include "plugins/accounts/accounts-manager.h"

#include <fmt/format.h>
#include <glib/gstdio.h>

#include <cinttypes>
#include <cstdint>

#include "lib/auth-manager.h"
#include "lib/helper.h"
#include "lib/log.h"
#include "plugins/accounts/accounts-common.h"
#include "plugins/accounts/accounts-util.h"

namespace Kiran
{
#define ACCOUNTS_OBJECT_PATH "/com/unikylin/Kiran/SystemDaemon/Accounts"

AccountsManager::AccountsManager(AccountsWrapper *passwd_wrapper) : passwd_wrapper_(passwd_wrapper),
                                                                    dbus_connect_id_(0),
                                                                    object_register_id_(0)
{
}

AccountsManager::~AccountsManager()
{
    if (this->dbus_connect_id_)
    {
        Gio::DBus::unown_name(this->dbus_connect_id_);
    }

    if (this->reload_conn_)
    {
        this->reload_conn_.disconnect();
    }
}

AccountsManager *AccountsManager::instance_ = nullptr;
void AccountsManager::global_init(AccountsWrapper *passwd_wrapper)
{
    instance_ = new AccountsManager(passwd_wrapper);
    instance_->init();
}

std::shared_ptr<User> AccountsManager::lookup_user_by_name(const std::string &user_name)
{
    auto iter = this->users_.find(user_name);
    if (iter != this->users_.end())
    {
        return iter->second;
    }
    return nullptr;
}

std::shared_ptr<User> AccountsManager::lookup_user_by_uid(int64_t uid)
{
    auto iter = this->users_by_uid_.find(uid);
    if (iter != this->users_by_uid_.end())
    {
        if (!iter->second.expired())
        {
            return iter->second.lock();
        }
    }
    return nullptr;
}

std::shared_ptr<User> AccountsManager::get_autologin_user()
{
    if (!this->autologin_.expired())
    {
        return this->autologin_.lock();
    }
    return nullptr;
}

bool AccountsManager::set_automatic_login(std::shared_ptr<User> user, bool enabled, std::string &err)
{
    std::shared_ptr<User> cur_autologin = this->get_autologin_user();

    RETURN_VAL_IF_TRUE(cur_autologin == user && enabled, true);
    RETURN_VAL_IF_TRUE(cur_autologin != user && !enabled, true);

    if (!this->save_autologin(user->UserName_get(), enabled, err))
    {
        err = fmt::format("failed to save autologin info to configuration file: {0}", err);
        return false;
    }

    if (cur_autologin)
    {
        cur_autologin->AutomaticLogin_set(false);
    }

    if (enabled)
    {
        user->AutomaticLogin_set(true);
        this->autologin_ = user;
    }

    return true;
}

void AccountsManager::ListCachedUsers(MethodInvocation &invocation)
{
    if (this->reload_conn_)
    {
        auto idle = Glib::MainContext::get_default()->signal_idle();
        idle.connect(sigc::bind(sigc::mem_fun(this, &AccountsManager::list_cached_users_idle), invocation));
    }
    else
    {
        this->list_cached_users_idle(invocation);
    }
}

void AccountsManager::FindUserById(guint64 uid, MethodInvocation &invocation)
{
    SETTINGS_PROFILE("uid: %" PRId64 " ", uid);

    auto user = this->find_and_create_user_by_id(uid);

    if (user)
    {
        invocation.ret(user->get_object_path());
    }
    else
    {
        auto err_message = fmt::format("Failed to look up user with id %" PRIu64 ".", uid);
        invocation.ret(Glib::Error(ACCOUNTS_ERROR, static_cast<int32_t>(AccountsError::ERROR_FAILED), err_message.c_str()));
    }

    return;
}

void AccountsManager::FindUserByName(const Glib::ustring &name, MethodInvocation &invocation)
{
    SETTINGS_PROFILE("name %s", name.c_str());

    auto user = this->find_and_create_user_by_name(name);

    if (user)
    {
        invocation.ret(user->get_object_path());
    }
    else
    {
        auto err_message = fmt::format("Failed to look up user with name %s.", name.c_str());
        invocation.ret(Glib::Error(ACCOUNTS_ERROR, static_cast<int32_t>(AccountsError::ERROR_FAILED), err_message.c_str()));
    }

    return;
}

void AccountsManager::CreateUser(const Glib::ustring &name,
                                 const Glib::ustring &real_name,
                                 gint32 account_type,
                                 MethodInvocation &invocation)
{
    SETTINGS_PROFILE("name :%s real_name: %s account_type: %d", name.c_str(), real_name.c_str(), account_type);

    AuthManager::get_instance()->start_auth_check(AUTH_USER_ADMIN,
                                                  TRUE,
                                                  invocation.getMessage(),
                                                  std::bind(&AccountsManager::create_user_authorized_cb, this, std::placeholders::_1, name, real_name, account_type));

    return;
}

void AccountsManager::DeleteUser(guint64 uid, bool remove_files, MethodInvocation &invocation)
{
    SETTINGS_PROFILE("uid: %" PRIu64 " remoev_files: %d", uid, remove_files);

    AuthManager::get_instance()->start_auth_check(AUTH_USER_ADMIN,
                                                  TRUE,
                                                  invocation.getMessage(),
                                                  std::bind(&AccountsManager::delete_user_authorized_cb, this, std::placeholders::_1, uid, remove_files));

    return;
}

void AccountsManager::init()
{
    SETTINGS_PROFILE("");

    this->dbus_connect_id_ = Gio::DBus::own_name(Gio::DBus::BUS_TYPE_SYSTEM,
                                                 ACCOUNTS_DBUS_NAME,
                                                 sigc::mem_fun(this, &AccountsManager::on_bus_acquired),
                                                 sigc::mem_fun(this, &AccountsManager::on_name_acquired),
                                                 sigc::mem_fun(this, &AccountsManager::on_name_lost));

    this->passwd_wrapper_->signal_file_changed().connect(sigc::mem_fun(this, &AccountsManager::file_changed));

    load_users();
}

void AccountsManager::file_changed(FileChangedType type)
{
    SETTINGS_PROFILE("");

    if (this->reload_conn_)
    {
        return;
    }

    auto timeout = Glib::MainContext::get_default()->signal_timeout();
    this->reload_conn_ = timeout.connect(sigc::mem_fun(this, &AccountsManager::file_changed_timeout), 500);
}

bool AccountsManager::file_changed_timeout()
{
    SETTINGS_PROFILE("");

    reload_users();

    return false;
}

bool AccountsManager::reload_users()
{
    SETTINGS_PROFILE("");

    auto new_users = load_users();
    int32_t number_of_normal_users = 0;

    //  wtmp_helper_update_login_frequencies(users);

    /* Count the non-system users. Mark which users are local, which are not. */
    for (auto iter = new_users.begin(); iter != new_users.end(); ++iter)
    {
        auto user = iter->second;
        if (!user->SystemAccount_get())
        {
            ++number_of_normal_users;
        }
        // user_update_local_account_property(user, g_hash_table_lookup(local, name) != NULL);
    }

    /* Remove all the old users */
    for (auto iter = this->users_.begin(); iter != this->users_.end(); ++iter)
    {
        auto iter2 = new_users.find(iter->first);
        if (iter2 == new_users.end())
        {
            this->UserAdded_signal.emit(iter->second->get_object_path());
            iter->second->dbus_unregister();
        }
    }

    /* Register all the new users */
    for (auto iter = new_users.begin(); iter != new_users.end(); ++iter)
    {
        auto iter2 = this->users_.find(iter->first);
        if (iter2 == this->users_.end())
        {
            iter->second->dbus_register();
            this->UserDeleted_signal.emit(iter->second->get_object_path());
        }
        iter->second->thaw_notify();
    }

    this->users_ = new_users;

    return true;
}

std::map<std::string, std::shared_ptr<User>> AccountsManager::load_users()
{
    std::map<std::string, std::shared_ptr<User>> users;

    auto passwds_shadows = AccountsWrapper::get_instance()->get_passwds_shadows();
    for (auto iter = passwds_shadows.begin(); iter != passwds_shadows.end(); ++iter)
    {
        std::shared_ptr<User> user;
        auto &pwent = iter->first;
        // auto &spent = iter->second;

        /* Skip system users... */
        if (!this->is_explicitly_requested_user(pwent->pw_name) &&
            !UserClassify::is_human(pwent->pw_uid, pwent->pw_name, pwent->pw_shell))
        {
            LOG_DEBUG("skipping user: %s", pwent->pw_name);
            continue;
        }

        auto user_iter = users.find(pwent->pw_name);

        if (user_iter == users.end())
        {
            auto user_iter2 = this->users_.find(pwent->pw_name);

            if (user_iter2 == this->users_.end())
            {
                user = std::make_shared<User>(pwent->pw_uid);
            }
            else
            {
                user = user_iter2->second;
            }

            /* freeze & update users not already in the new list */
            user->freeze_notify();
            user->update_from_passwd_shadow(*iter);

            users.emplace(user->UserName_get(), user);
            LOG_DEBUG("loaded user: %s", user->UserName_get().c_str());
        }
        else
        {
            user = user_iter->second;
        }

        if (!this->is_explicitly_requested_user(pwent->pw_name))
        {
            user->set_cached(true);
        }
    }
    return users;
}

std::shared_ptr<User> AccountsManager::add_new_user_for_pwent(std::shared_ptr<Passwd> pwent, std::shared_ptr<SPwd> spent)
{
    auto user = std::make_shared<User>(pwent->pw_uid);
    user->update_from_passwd_shadow(std::make_pair(pwent, spent));
    user->dbus_register();
    auto iter = this->users_.emplace(user->UserName_get(), user);
    if (!iter.second)
    {
        LOG_WARNING("user %s is already exist.", user->UserName_get().c_str());
        return iter.first->second;
    }
    else
    {
        this->UserAdded_signal.emit(user->get_object_path());
        return user;
    }
}

std::shared_ptr<User> AccountsManager::find_and_create_user_by_id(uint64_t uid)
{
    auto pwent = AccountsWrapper::get_instance()->get_passwd_by_uid(uid);
    if (!pwent)
    {
        LOG_DEBUG("unable to lookup uid %u", (uint32_t)uid);
        return nullptr;
    }

    auto user = this->lookup_user_by_uid(uid);
    if (!user)
    {
        auto spent = AccountsWrapper::get_instance()->get_spwd_by_name(pwent->pw_name);
        user = this->add_new_user_for_pwent(pwent, spent);
        this->explicitly_requested_users_.insert(pwent->pw_name);
    }

    return user;
}

std::shared_ptr<User> AccountsManager::find_and_create_user_by_name(const std::string &user_name)
{
    auto pwent = AccountsWrapper::get_instance()->get_passwd_by_name(user_name);
    if (!pwent)
    {
        LOG_DEBUG("unable to lookup name %s", user_name.c_str());
        return nullptr;
    }

    auto user = this->lookup_user_by_name(user_name);
    if (!user)
    {
        auto spent = AccountsWrapper::get_instance()->get_spwd_by_name(pwent->pw_name);
        user = this->add_new_user_for_pwent(pwent, spent);
        this->explicitly_requested_users_.insert(pwent->pw_name);
    }

    return user;
}

bool AccountsManager::list_cached_users_idle(MethodInvocation invocation)
{
    std::vector<Glib::DBusObjectPathString> cached_users;
    for (auto iter = this->users_.begin(); iter != this->users_.end(); ++iter)
    {
        if (iter->second->get_cached())
        {
            cached_users.push_back(iter->second->get_object_path());
        }
    }
    invocation.ret(cached_users);
    return false;
}

void AccountsManager::create_user_authorized_cb(MethodInvocation invocation,
                                                const Glib::ustring &name,
                                                const Glib::ustring &fullname,
                                                gint32 account_type)
{
    auto pwent = AccountsWrapper::get_instance()->get_passwd_by_name(name);

    if (pwent)
    {
        auto err_message = fmt::format("A user with name '{0}' already exists", name.raw());
        invocation.ret(Glib::Error(ACCOUNTS_ERROR, static_cast<int32_t>(AccountsError::ERROR_USER_EXISTS), err_message.c_str()));
        return;
    }

    LOG_DEBUG("create user '%s'", name.c_str());

    std::vector<std::string> argv;
    switch (account_type)
    {
        case int32_t(AccountType::ACCOUNT_TYPE_ADMINISTRATOR):
            argv = std::vector<std::string>({"/usr/sbin/useradd", "-m", "-c", fullname.raw(), "-G", ADMIN_GROUP, "--", name.raw()});
            break;
        case int32_t(AccountType::ACCOUNT_TYPE_STANDARD):
            argv = std::vector<std::string>({"/usr/sbin/useradd", "-m", "-c", fullname.raw(), "--", name.raw()});
            break;
        default:
        {
            auto err_message = fmt::format("Don't know how to add user of type %d", int32_t(account_type));
            invocation.ret(Glib::Error(ACCOUNTS_ERROR, static_cast<int32_t>(AccountsError::ERROR_FAILED), err_message.c_str()));
            return;
        }
        break;
    }

    std::string err;
    if (!AccountsUtil::spawn_with_login_uid(invocation.getMessage(), argv, err))
    {
        auto err_message = fmt::format("running '{0}' failed: {1}", argv[0], err);
        invocation.ret(Glib::Error(ACCOUNTS_ERROR, static_cast<int32_t>(AccountsError::ERROR_FAILED), err_message.c_str()));
        return;
    }

    auto user = this->find_and_create_user_by_name(name);
    if (user)
    {
        user->LocalAccount_set(true);
        user->SystemAccount_set(false);
        user->save_data();
        invocation.ret(user->get_object_path());
    }
    else
    {
        auto err_message = fmt::format("failed to find or create user {0}.", name.raw());
        invocation.ret(Glib::Error(ACCOUNTS_ERROR, static_cast<int32_t>(AccountsError::ERROR_FAILED), err_message.c_str()));
    }
}

void AccountsManager::delete_user_authorized_cb(MethodInvocation invocation, uint64_t uid, bool remove_files)
{
    if (uid == 0)
    {
        invocation.ret(Glib::Error(ACCOUNTS_ERROR, static_cast<int32_t>(AccountsError::ERROR_FAILED), "Refuse to delete root user"));
        return;
    }

    std::string err;
    auto user = this->find_and_create_user_by_id(uid);

    if (!user)
    {
        auto err_message = fmt::format("No user with uid {0} found", uid);
        invocation.ret(Glib::Error(ACCOUNTS_ERROR, static_cast<int32_t>(AccountsError::ERROR_USER_DOES_NOT_EXIST), err_message.c_str()));
        return;
    }

    LOG_DEBUG("delete user '%s' (%d)", user->UserName_get().c_str(), (int32_t)uid);

    user->set_cached(false);

    if (this->get_autologin_user() == user)
    {
        if (!this->set_automatic_login(user, false, err))
        {
            LOG_WARNING("%s", err.c_str());
        }
    }

    this->remove_cache_files(user->UserName_get());
    user->Saved_set(false);

    std::vector<std::string> argv;

    if (remove_files)
    {
        argv = std::vector<std::string>({"/usr/sbin/userdel", "-f", "-r", "--", user->UserName_get().raw()});
    }
    else
    {
        argv = std::vector<std::string>({"/usr/sbin/userdel", "-f", "--", user->UserName_get().raw()});
    }

    if (!AccountsUtil::spawn_with_login_uid(invocation.getMessage(), argv, err))
    {
        auto err_message = fmt::format("running '{0}' failed: {1}", argv[0], err);
        invocation.ret(Glib::Error(ACCOUNTS_ERROR, static_cast<int32_t>(AccountsError::ERROR_FAILED), err_message.c_str()));
        return;
    }

    invocation.ret();
}

void AccountsManager::remove_cache_files(const std::string &user_name)
{
    auto user_filename = Glib::build_filename(USERDIR, user_name);
    g_remove(user_filename.c_str());

    auto icon_filename = Glib::build_filename(ICONDIR, user_name);
    g_remove(icon_filename.c_str());
}

bool AccountsManager::is_explicitly_requested_user(const std::string &user_name)
{
    return (this->explicitly_requested_users_.find(user_name) != this->explicitly_requested_users_.end());
}

bool AccountsManager::load_autologin(std::string &name, bool &enabled, std::string &err)
{
    Glib::KeyFile keyfile;

    try
    {
        keyfile.load_from_file(PATH_GDM_CUSTOM, Glib::KEY_FILE_KEEP_COMMENTS);
    }
    catch (const Glib::Error &e)
    {
        err = e.what().raw();
        return false;
    }

    try
    {
        auto enabled_value = keyfile.get_string("daemon", "AutomaticLoginEnable");
        enabled_value = str_tolower(enabled_value);
        if (enabled_value == "true" || enabled_value == "1")
        {
            enabled = true;
        }
        else
        {
            enabled = false;
        }
        name = keyfile.get_string("daemon", "AutomaticLogin");
    }
    catch (const Glib::Error &e)
    {
        err = e.what().raw();
        return false;
    }

    return true;
}

bool AccountsManager::save_autologin(const std::string &name, bool enabled, std::string &err)
{
    Glib::KeyFile keyfile;
    try
    {
        keyfile.load_from_file(PATH_GDM_CUSTOM, Glib::KEY_FILE_KEEP_COMMENTS);
    }
    catch (const Glib::Error &e)
    {
        err = e.what().raw();
        return false;
    }

    keyfile.set_string("daemon", "AutomaticLoginEnable", enabled ? "True" : "False");
    keyfile.set_string("daemon", "AutomaticLogin", name);

    auto data = keyfile.to_data();
    try
    {
        Glib::file_set_contents(PATH_GDM_CUSTOM, data);
    }
    catch (const Glib::Error &e)
    {
        err = e.what().raw();
        return false;
    }

    return true;
}

void AccountsManager::on_bus_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name)
{
    if (!connect)
    {
        LOG_WARNING("failed to connect dbus. name: %s", name.c_str());
        return;
    }
    try
    {
        this->object_register_id_ = this->register_object(connect, ACCOUNTS_OBJECT_PATH);
    }
    catch (const Glib::Error &e)
    {
        LOG_WARNING("register object_path %s fail: %s.", ACCOUNTS_OBJECT_PATH, e.what().c_str());
    }
}

void AccountsManager::on_name_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name)
{
    LOG_DEBUG("success to register dbus name: %s", name.c_str());
}

void AccountsManager::on_name_lost(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name)
{
    LOG_WARNING("failed to register dbus name: %s", name.c_str());
}

}  // namespace Kiran
