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
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
 */

#include "plugins/accounts/accounts-manager.h"

#include <glib/gstdio.h>

#include <cinttypes>
#include <cstdint>

#include "accounts-i.h"
#include "lib/base/base.h"
#include "lib/base/crypto-helper.h"
#include "lib/dbus/dbus.h"
#include "plugins/accounts/accounts-util.h"

namespace Kiran
{
// 最少需要512长度
#define RSA_KEY_LENGTH 512
#define PATH_GDM_CUSTOM "/etc/gdm/custom.conf"

#define LOGIN1_DBUS_NAME "org.freedesktop.login1"
#define LOGIN1_DBUS_OBJECT_PATH "/org/freedesktop/login1"
#define LOGIN1_MANAGER_DBUS_INTERFACE "org.freedesktop.login1.Manager"

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
    KLOG_DEBUG_ACCOUNTS("Unable to look up user %s", user_name.c_str());
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

bool AccountsManager::set_automatic_login(std::shared_ptr<User> user, bool enabled, CCErrorCode &error_code)
{
    std::shared_ptr<User> cur_autologin = this->get_autologin_user();

    RETURN_VAL_IF_TRUE(cur_autologin == user && enabled, true);
    RETURN_VAL_IF_TRUE(cur_autologin != user && !enabled, true);
    auto user_name = user ? user->user_name_get().raw() : std::string();

    std::string error;
    if (!this->save_autologin_to_file(user_name, enabled, error))
    {
        KLOG_WARNING_ACCOUNTS("%s", error.c_str());
        error_code = CCErrorCode::ERROR_ACCOUNTS_SAVE_AUTOLOGIN_FILE;
        return false;
    }

    if (cur_autologin)
    {
        cur_autologin->automatic_login_set(false);
    }

    user->automatic_login_set(enabled);
    this->autologin_ = enabled ? user : nullptr;

    return true;
}

void AccountsManager::GetNonSystemUsers(MethodInvocation &invocation)
{
    // 如果正在加载用户信息，则延时获取非系统用户列表
    if (this->reload_conn_)
    {
        auto idle = Glib::MainContext::get_default()->signal_idle();
        idle.connect(sigc::bind(sigc::mem_fun(this, &AccountsManager::list_non_system_users_idle), invocation));
    }
    else
    {
        this->list_non_system_users_idle(invocation);
    }
}

void AccountsManager::FindUserById(guint64 uid, MethodInvocation &invocation)
{
    auto user = this->find_and_create_user_by_id(uid);

    if (user)
    {
        invocation.ret(user->get_object_path());
    }
    else
    {
        DBUS_ERROR_REPLY(CCErrorCode::ERROR_ACCOUNTS_USER_NOT_FOUND_1);
    }

    return;
}

void AccountsManager::FindUserByName(const Glib::ustring &name, MethodInvocation &invocation)
{
    auto user = this->find_and_create_user_by_name(name);

    if (user)
    {
        invocation.ret(user->get_object_path());
    }
    else
    {
        DBUS_ERROR_REPLY(CCErrorCode::ERROR_ACCOUNTS_USER_NOT_FOUND_2);
    }

    return;
}

void AccountsManager::FindUserByAuthData(gint32 mode, const Glib::ustring &data_id, MethodInvocation &invocation)
{
    UserVec users;
    for (auto iter : this->users_)
    {
        if (iter.second->match_auth_data(mode, data_id))
        {
            users.push_back(iter.second);
        }
    }

    if (users.size() == 0)
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_ACCOUNTS_USER_NOT_FOUND_5);
    }
    else if (users.size() == 1)
    {
        invocation.ret(users[0]->get_object_path());
    }
    else
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_ACCOUNTS_AUTH_DATA_CONFLICT);
    }
}

void AccountsManager::CreateUser(const Glib::ustring &name,
                                 const Glib::ustring &real_name,
                                 gint32 account_type,
                                 gint64 uid,
                                 MethodInvocation &invocation)
{
    KLOG_DEBUG_ACCOUNTS("Create an user and the name is %s and real_name is %s, account_type is %d, uid is %" PRIu64 ".",
                        name.c_str(),
                        real_name.c_str(),
                        account_type,
                        uid);

    AuthManager::get_instance()->start_auth_check(AUTH_USER_ADMIN,
                                                  TRUE,
                                                  invocation.getMessage(),
                                                  std::bind(&AccountsManager::create_user_authorized_cb, this, std::placeholders::_1, name, real_name, account_type, uid));

    return;
}

void AccountsManager::DeleteUser(guint64 uid, bool remove_files, MethodInvocation &invocation)
{
    KLOG_DEBUG_ACCOUNTS("Delete the user with Uid %" PRIu64 "remove_files: %d", uid, remove_files);

    // 如果是三权用户，则禁止删除
    if (this->is_security_policy_user(uid))
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_ACCOUNTS_DELETE_THREE_AUTH_USER);
    }

    // 如果用户已经登录, 则禁止删除
    if (this->login1_proxy_)
    {
        auto parameters = g_variant_new("(u)", uint32_t(uid));
        Glib::VariantContainerBase base(parameters, false);
        try
        {
            auto retval = this->login1_proxy_->call_sync("GetUser", base);
            auto v1 = retval.get_child(0);
            auto user_object_path = Glib::VariantBase::cast_dynamic<Glib::Variant<Glib::DBusObjectPathString>>(v1).get();

            if (!user_object_path.empty())
            {
                DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_ACCOUNTS_ALREADY_LOGIN);
            }
        }
        catch (const std::exception &e)
        {
            KLOG_DEBUG_ACCOUNTS("%s.", e.what());
        }
        catch (const Glib::Error &e)
        {
            KLOG_DEBUG_ACCOUNTS("%s.", e.what().c_str());
        }
    }

    AuthManager::get_instance()->start_auth_check(AUTH_USER_ADMIN,
                                                  TRUE,
                                                  invocation.getMessage(),
                                                  std::bind(&AccountsManager::delete_user_authorized_cb, this, std::placeholders::_1, uid, remove_files));

    return;
}

bool AccountsManager::rsa_public_key_setHandler(const Glib::ustring &value)
{
    KLOG_DEBUG_ACCOUNTS("Unsupported operation.");
    return false;
}

void AccountsManager::init()
{
    try
    {
        this->login1_proxy_ = Gio::DBus::Proxy::create_for_bus_sync(Gio::DBus::BUS_TYPE_SYSTEM,
                                                                    LOGIN1_DBUS_NAME,
                                                                    LOGIN1_DBUS_OBJECT_PATH,
                                                                    LOGIN1_MANAGER_DBUS_INTERFACE);
    }
    catch (const Glib::Error &e)
    {
        KLOG_WARNING_ACCOUNTS("%s.", e.what().c_str());
    }

    this->dbus_connect_id_ = Gio::DBus::own_name(Gio::DBus::BUS_TYPE_SYSTEM,
                                                 ACCOUNTS_DBUS_NAME,
                                                 sigc::mem_fun(this, &AccountsManager::on_bus_acquired),
                                                 sigc::mem_fun(this, &AccountsManager::on_name_acquired),
                                                 sigc::mem_fun(this, &AccountsManager::on_name_lost));

    this->passwd_wrapper_->signal_file_changed().connect(sigc::mem_fun(this, &AccountsManager::accounts_file_changed));
    this->gdm_custom_monitor_ = FileUtils::make_monitor_file(PATH_GDM_CUSTOM, sigc::mem_fun(this, &AccountsManager::gdm_custom_changed));

    CryptoHelper::generate_rsa_key(RSA_KEY_LENGTH, this->rsa_private_key_, this->rsa_public_key_);

    reload_users();
    update_automatic_login();
}

void AccountsManager::accounts_file_changed(FileChangedType type)
{
    if (this->reload_conn_)
    {
        return;
    }

    auto timeout = Glib::MainContext::get_default()->signal_timeout();
    this->reload_conn_ = timeout.connect(sigc::mem_fun(this, &AccountsManager::accounts_file_changed_timeout), 500);
}

bool AccountsManager::accounts_file_changed_timeout()
{
    reload_users();

    return false;
}

void AccountsManager::gdm_custom_changed(const Glib::RefPtr<Gio::File> &file, const Glib::RefPtr<Gio::File> &other_file, Gio::FileMonitorEvent event_type)
{
    RETURN_IF_TRUE(event_type != Gio::FILE_MONITOR_EVENT_CHANGED &&
                   event_type != Gio::FILE_MONITOR_EVENT_CREATED &&
                   event_type != Gio::FILE_MONITOR_EVENT_DELETED);

    this->update_automatic_login();
    return;
}

void AccountsManager::update_automatic_login()
{
    std::string name;
    bool enabled;
    std::string err;
    if (!this->read_autologin_from_file(name, enabled, err))
    {
        KLOG_WARNING_ACCOUNTS("Failed to load gdms custom.conf: %s.", err.c_str());
        return;
    }
    std::shared_ptr<User> user;
    if (name.length() > 0)
    {
        user = this->find_and_create_user_by_name(name);
    }

    CCErrorCode error_code = CCErrorCode::SUCCESS;
    this->set_automatic_login(user, enabled, error_code);
}

bool AccountsManager::reload_users()
{
    auto new_users = load_users();
    int32_t number_of_normal_users = 0;

    //  wtmp_helper_update_login_frequencies(users);

    /* Count the non-system users. Mark which users are local, which are not. */
    for (auto iter = new_users.begin(); iter != new_users.end(); ++iter)
    {
        auto user = iter->second;
        if (!user->system_account_get())
        {
            ++number_of_normal_users;
        }
        // user_update_local_account_property(user, g_hash_table_lookup(local, name)
        // != NULL);
    }

    /* Remove all the old users */
    for (auto iter = this->users_.begin(); iter != this->users_.end(); ++iter)
    {
        auto iter2 = new_users.find(iter->first);
        if (iter2 == new_users.end())
        {
            this->UserDeleted_signal.emit(iter->second->get_object_path());
            iter->second->dbus_unregister();
            iter->second->remove_cache_file();
        }
    }

    /* Register all the new users */
    for (auto iter = new_users.begin(); iter != new_users.end(); ++iter)
    {
        auto iter2 = this->users_.find(iter->first);
        if (iter2 == this->users_.end())
        {
            iter->second->dbus_register();
            this->UserAdded_signal.emit(iter->second->get_object_path());
        }
    }

    this->users_ = new_users;

    return true;
}

std::map<std::string, std::shared_ptr<User>> AccountsManager::load_users()
{
    auto passwds_shadows = this->passwd_wrapper_->get_passwds_shadows();
    std::map<std::string, std::shared_ptr<User>> users;

    for (auto iter = passwds_shadows.begin(); iter != passwds_shadows.end(); ++iter)
    {
        std::shared_ptr<User> user;
        auto pwent = iter->first;

        // 除了root用户和通过DBUS显示请求的系统用户以外，其他系统用户默认不加载。
        if (!this->is_explicitly_requested_user(pwent->pw_name) &&
            !UserClassify::is_human(pwent->pw_uid, pwent->pw_name, pwent->pw_shell) &&
            pwent->pw_uid != 0)
        {
            KLOG_DEBUG_ACCOUNTS("Skip user: %s.", pwent->pw_name.c_str());
            continue;
        }

        auto old_iter = this->users_.find(pwent->pw_name);

        if (old_iter == this->users_.end())
        {
            user = User::create_user(*iter);
        }
        else
        {
            user = old_iter->second;
            user->update_from_passwd_shadow(*iter);
        }

        auto new_iter = users.emplace(user->user_name_get().raw(), user);

        if (!new_iter.second)
        {
            KLOG_WARNING_ACCOUNTS("Exist same user_name: %s.", pwent->pw_name.c_str());
        }
        else
        {
            KLOG_DEBUG_ACCOUNTS("Add user %s.", pwent->pw_name.c_str());
        }
    }
    return users;
}

std::shared_ptr<User> AccountsManager::add_new_user_for_pwent(std::shared_ptr<Passwd> pwent, std::shared_ptr<SPwd> spent)
{
    KLOG_DEBUG_ACCOUNTS("Add new user %s.", pwent->pw_name.c_str());

    auto user = User::create_user(std::make_pair(pwent, spent));
    user->dbus_register();
    auto iter = this->users_.emplace(user->user_name_get(), user);
    if (!iter.second)
    {
        KLOG_WARNING_ACCOUNTS("User %s is already exist.", user->user_name_get().c_str());
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
    auto pwent = this->passwd_wrapper_->get_passwd_by_uid(uid);
    if (!pwent)
    {
        return nullptr;
    }

    auto user = this->lookup_user_by_name(pwent->pw_name);
    if (!user)
    {
        KLOG_DEBUG_ACCOUNTS("Unable to lookup user by uid %d.", uid);
        auto spent = this->passwd_wrapper_->get_spwd_by_name(pwent->pw_name);
        user = this->add_new_user_for_pwent(pwent, spent);
        this->explicitly_requested_users_.insert(pwent->pw_name);
    }

    return user;
}

std::shared_ptr<User> AccountsManager::find_and_create_user_by_name(const std::string &user_name)
{
    auto pwent = this->passwd_wrapper_->get_passwd_by_name(user_name);
    if (!pwent)
    {
        KLOG_DEBUG_ACCOUNTS("Unable to lookup user name %s.", user_name.c_str());
        return nullptr;
    }

    auto user = this->lookup_user_by_name(user_name);
    if (!user)
    {
        KLOG_DEBUG_ACCOUNTS("Unable to lookup user name %s.", user_name.c_str());
        auto spent = this->passwd_wrapper_->get_spwd_by_name(pwent->pw_name);
        user = this->add_new_user_for_pwent(pwent, spent);
        this->explicitly_requested_users_.insert(pwent->pw_name);
    }

    return user;
}

bool AccountsManager::list_non_system_users_idle(MethodInvocation invocation)
{
    std::vector<Glib::DBusObjectPathString> non_system_users;
    for (auto iter = this->users_.begin(); iter != this->users_.end(); ++iter)
    {
        if (!iter->second->system_account_get())
        {
            non_system_users.push_back(iter->second->get_object_path());
        }
    }
    invocation.ret(non_system_users);
    return false;
}

void AccountsManager::create_user_authorized_cb(MethodInvocation invocation,
                                                const Glib::ustring &name,
                                                const Glib::ustring &realname,
                                                gint32 account_type,
                                                gint64 uid)
{
    auto pwent = this->passwd_wrapper_->get_passwd_by_name(name);

    if (pwent)
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_ACCOUNTS_USER_ALREADY_EXIST);
    }

    KLOG_DEBUG_ACCOUNTS("Create user '%s'.", name.c_str());

    std::vector<std::string> argv = {"/usr/sbin/useradd", "-m", "-c", realname.raw()};
    switch (account_type)
    {
    case int32_t(AccountsAccountType::ACCOUNTS_ACCOUNT_TYPE_ADMINISTRATOR):
        argv.insert(argv.end(), {"-G", ADMIN_GROUP});
        break;
    case int32_t(AccountsAccountType::ACCOUNTS_ACCOUNT_TYPE_STANDARD):
        break;
    default:
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_ACCOUNTS_UNKNOWN_ACCOUNT_TYPE);
    }
    break;
    }

    if (uid > 0)
    {
        argv.insert(argv.end(), {"-u", fmt::format("{0}", uid)});
    }

    argv.insert(argv.end(), {"--", name.raw()});
    SPAWN_DBUS_WITH_ARGS(invocation, argv);

    auto user = this->find_and_create_user_by_name(name);
    if (user)
    {
        user->system_account_set(false);
        invocation.ret(user->get_object_path());
    }
    else
    {
        DBUS_ERROR_REPLY(CCErrorCode::ERROR_ACCOUNTS_USER_NOT_FOUND_3);
    }
}

void AccountsManager::delete_user_authorized_cb(MethodInvocation invocation, uint64_t uid, bool remove_files)
{
    KLOG_DEBUG_ACCOUNTS("Delete authorized user with uid %" PRIu64 " remoev_files: %d", uid, remove_files);
    if (uid == 0)
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_ACCOUNTS_DELETE_ROOT_USER);
    }

    CCErrorCode error_code = CCErrorCode::SUCCESS;
    auto user = this->find_and_create_user_by_id(uid);

    if (!user)
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_ACCOUNTS_USER_NOT_FOUND_4);
    }

    KLOG_DEBUG_ACCOUNTS("Delete user '%s' (%d)", user->user_name_get().c_str(), (int32_t)uid);

    // 忽略取消自动登陆出错情况
    this->set_automatic_login(user, false, error_code);
    user->remove_cache_file();

    std::vector<std::string> argv;

    if (remove_files)
    {
        argv = std::vector<std::string>({"/usr/sbin/userdel", "-f", "-r", "--", user->user_name_get().raw()});
    }
    else
    {
        argv = std::vector<std::string>({"/usr/sbin/userdel", "-f", "--", user->user_name_get().raw()});
    }

    SPAWN_DBUS_WITH_ARGS(invocation, argv);
    invocation.ret();
}

bool AccountsManager::is_explicitly_requested_user(const std::string &user_name)
{
    return (this->explicitly_requested_users_.find(user_name) != this->explicitly_requested_users_.end());
}

bool AccountsManager::is_security_policy_user(uint64_t uid)
{
    auto user = this->find_and_create_user_by_id(uid);
    if (user)
    {
        if (user->user_name_get().raw() == "audadm" ||
            user->user_name_get().raw() == "sysadm" ||
            user->user_name_get().raw() == "secadm")
        {
            return true;
        }
    }

    return false;
}

bool AccountsManager::read_autologin_from_file(std::string &name, bool &enabled, std::string &err)
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
        enabled_value = StrUtils::tolower(enabled_value);
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

bool AccountsManager::save_autologin_to_file(const std::string &name, bool enabled, std::string &err)
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

    if (!name.empty())
    {
        keyfile.set_string("daemon", "AutomaticLoginEnable", enabled ? "True" : "False");
        keyfile.set_string("daemon", "AutomaticLogin", name);
    }
    else
    {
        try
        {
            keyfile.remove_key("daemon", "AutomaticLoginEnable");
            keyfile.remove_key("daemon", "AutomaticLogin");
        }
        catch (const Glib::Error &e)
        {
            err = e.what().raw();
            return false;
        }
    }

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
        KLOG_WARNING_ACCOUNTS("Failed to connect dbus with %s.", name.c_str());
        return;
    }
    try
    {
        this->object_register_id_ = this->register_object(connect, ACCOUNTS_OBJECT_PATH);
    }
    catch (const Glib::Error &e)
    {
        KLOG_WARNING_ACCOUNTS("Register object_path %s fail: %s.", ACCOUNTS_OBJECT_PATH, e.what().c_str());
    }
}

void AccountsManager::on_name_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name)
{
    KLOG_DEBUG_ACCOUNTS("Success to register dbus name: %s.", name.c_str());
}

void AccountsManager::on_name_lost(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name)
{
    KLOG_WARNING_ACCOUNTS("Failed to register dbus name: %s.", name.c_str());
}

}  // namespace Kiran
