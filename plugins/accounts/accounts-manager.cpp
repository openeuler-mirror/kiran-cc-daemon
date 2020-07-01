/*
 * @Author       : tangjie02
 * @Date         : 2020-06-19 10:09:05
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-07-01 14:03:11
 * @Description  : 
 * @FilePath     : /kiran-system-daemon/plugins/accounts/accounts-manager.cpp
 */

#include "plugins/accounts/accounts-manager.h"

#include <act/act-user-manager.h>
#include <fmt/format.h>

#include "lib/helper.h"
#include "lib/log.h"
#include "plugins/accounts/accounts-common.h"

namespace Kiran
{
AccountsManager::AccountsManager()
{
    act_user_manager_ = act_user_manager_get_default();
}

AccountsManager *AccountsManager::instance_ = nullptr;
void AccountsManager::global_init()
{
    instance_ = new AccountsManager();
    instance_->init();
}

void AccountsManager::CreateUser(const Glib::ustring &name,
                                 const Glib::ustring &real_name,
                                 gint32 account_type,
                                 MethodInvocation &invocation)
{
    g_autoptr(GError) error = NULL;
    auto act_user = act_user_manager_create_user(this->act_user_manager_, name.c_str(), real_name.c_str(), ActUserAccountType(account_type), &error);

    if (act_user)
    {
        Glib::DBusObjectPathString object_path = act_user_get_object_path(act_user);
        invocation.ret(object_path);
    }
    else
    {
        invocation.ret(Glib::Error(ACCOUNTS_ERROR, static_cast<int32_t>(AccountsError::ERROR_FAILED), error->message));
    }
}

void AccountsManager::DeleteUser(gint64 id,
                                 bool remove_files,
                                 MethodInvocation &invocation)
{
    g_autoptr(GError) error = NULL;
    auto act_user = act_user_manager_get_user_by_id(this->act_user_manager_, id);
    auto ret = act_user_manager_delete_user(this->act_user_manager_, act_user, remove_files, &error);

    if (ret)
    {
        invocation.ret();
    }
    else
    {
        invocation.ret(Glib::Error(ACCOUNTS_ERROR, static_cast<int32_t>(AccountsError::ERROR_FAILED), error->message));
    }
}

void AccountsManager::FindUserById(gint64 id,
                                   MethodInvocation &invocation)
{
    auto act_user = act_user_manager_get_user_by_id(this->act_user_manager_, id);
    if (act_user)
    {
        Glib::DBusObjectPathString object_path = act_user_get_object_path(act_user);
        invocation.ret(object_path);
    }
    else
    {
        auto message = fmt::format("invalid id: {0}", id);
        invocation.ret(Glib::Error(ACCOUNTS_ERROR, static_cast<int32_t>(AccountsError::ERROR_FAILED), message.c_str()));
    }
}

void AccountsManager::FindUserByName(const Glib::ustring &name,
                                     MethodInvocation &invocation)
{
    auto act_user = act_user_manager_get_user(this->act_user_manager_, name.c_str());
    if (act_user)
    {
        Glib::DBusObjectPathString object_path = act_user_get_object_path(act_user);
        invocation.ret(object_path);
    }
    else
    {
        auto message = fmt::format("invalid name: {0}", name.raw());
        invocation.ret(Glib::Error(ACCOUNTS_ERROR, static_cast<int32_t>(AccountsError::ERROR_FAILED), message.c_str()));
    }
}

void AccountsManager::signal_user_added(ActUser *act_user, gpointer user_data)
{
    AccountsManager *self = (AccountsManager *)user_data;
    g_return_if_fail(self == AccountsManager::get_instance());

    self->add_user(act_user);

    Glib::DBusObjectPathString object_path = act_user_get_object_path(act_user);

    self->UserAdded_signal.emit(object_path);
}

void AccountsManager::signal_user_removed(ActUser *act_user, gpointer user_data)
{
    AccountsManager *self = (AccountsManager *)user_data;
    g_return_if_fail(self == AccountsManager::get_instance());

    self->del_user(act_user);

    Glib::DBusObjectPathString object_path = act_user_get_object_path(act_user);

    self->UserDeleted_signal.emit(object_path);
}

void AccountsManager::init()
{
    g_return_if_fail(this->act_user_manager_ != NULL);

    load_users();

    g_signal_connect(this->act_user_manager_, "user-added", G_CALLBACK(AccountsManager::signal_user_added), this);
    g_signal_connect(this->act_user_manager_, "user-removed", G_CALLBACK(AccountsManager::signal_user_removed), this);
}

void AccountsManager::load_users()
{
    SETTINGS_PROFILE("");

    auto users = act_user_manager_list_users(this->act_user_manager_);
    SCOPE_EXIT({ g_slist_free(users); });

    for (auto l = users; l != NULL; l = l->next)
    {
        auto act_user = (ActUser *)(l->data);
        add_user(act_user);
    }
}

void AccountsManager::add_user(ActUser *act_user)
{
    std::string user_name = act_user_get_user_name(act_user);
    if (user_name.length() > 0)
    {
        auto user = std::make_shared<User>(act_user);
        auto iter = this->users_.emplace(user_name, user);
        if (!iter.second)
        {
            LOG_WARNING("fail to inser user %s, it's already exist.", user_name.c_str());
        }
    }
    else
    {
        LOG_WARNING("user name is empty.");
    }
}

void AccountsManager::del_user(ActUser *act_user)
{
    std::string user_name = act_user_get_user_name(act_user);

    auto iter = this->users_.find(user_name);
    if (iter == this->users_.end())
    {
        LOG_WARNING("user %s isn't exist.", user_name.c_str());
    }
    else
    {
        this->users_.erase(iter);
    }
}

}  // namespace Kiran
