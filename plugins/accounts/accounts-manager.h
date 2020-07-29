/*
 * @Author       : tangjie02
 * @Date         : 2020-06-19 10:08:59
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-07-29 17:48:29
 * @Description  : 
 * @FilePath     : /kiran-system-daemon/plugins/accounts/accounts-manager.h
 */
#pragma once

#include <accounts_dbus_stub.h>

#include "plugins/accounts/accounts-wrapper.h"
#include "plugins/accounts/user-classify.h"
#include "plugins/accounts/user.h"

namespace Kiran
{
class AccountsManager : public SystemDaemon::AccountsStub
{
public:
    AccountsManager() = delete;
    AccountsManager(AccountsWrapper *passwd_wrapper);
    virtual ~AccountsManager();

    static AccountsManager *get_instance() { return instance_; };

    static void global_init(AccountsWrapper *passwd_wrapper);

    static void global_deinit() { delete instance_; };

    std::shared_ptr<User> lookup_user_by_name(const std::string &user_name);
    std::shared_ptr<User> lookup_user_by_uid(int64_t uid);
    std::shared_ptr<User> get_autologin_user();

    bool set_automatic_login(std::shared_ptr<User> user, bool enabled, std::string &err);

protected:
    virtual void ListCachedUsers(MethodInvocation &invocation);
    virtual void FindUserById(guint64 uid, MethodInvocation &invocation);
    virtual void FindUserByName(const Glib::ustring &name, MethodInvocation &invocation);
    virtual void CreateUser(const Glib::ustring &name, const Glib::ustring &fullname, gint32 account_type, MethodInvocation &invocation);
    virtual void DeleteUser(guint64 uid, bool remove_files, MethodInvocation &invocation);

private:
    void init();

    void accounts_file_changed(FileChangedType type);
    bool accounts_file_changed_timeout();
    void gdm_custom_changed(const Glib::RefPtr<Gio::File> &file, const Glib::RefPtr<Gio::File> &other_file, Gio::FileMonitorEvent event_type);
    void update_automatic_login();

    bool reload_users();
    std::map<std::string, std::shared_ptr<User>> load_users();

    std::shared_ptr<User> add_new_user_for_pwent(std::shared_ptr<Passwd> pwent, std::shared_ptr<SPwd> spent);
    std::shared_ptr<User> find_and_create_user_by_id(uint64_t uid);
    std::shared_ptr<User> find_and_create_user_by_name(const std::string &user_name);
    bool list_cached_users_idle(MethodInvocation invocation);
    void create_user_authorized_cb(MethodInvocation invocation, const Glib::ustring &name, const Glib::ustring &fullname, gint32 account_type);
    void delete_user_authorized_cb(MethodInvocation invocation, uint64_t uid, bool remove_files);

    void remove_cache_files(const std::string &user_name);
    bool is_explicitly_requested_user(const std::string &user_name);

    bool read_autologin_from_file(std::string &name, bool &enabled, std::string &err);
    bool save_autologin_to_file(const std::string &name, bool enabled, std::string &err);

    void on_bus_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name);
    void on_name_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name);
    void on_name_lost(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name);

private:
    static AccountsManager *instance_;

    AccountsWrapper *passwd_wrapper_;

    uint32_t dbus_connect_id_;
    uint32_t object_register_id_;
    sigc::connection reload_conn_;

    Glib::RefPtr<Gio::FileMonitor> gdm_custom_monitor_;

    std::map<std::string, std::shared_ptr<User>> users_;
    std::weak_ptr<User> autologin_;
    std::map<int64_t, std::weak_ptr<User>> users_by_uid_;
    std::set<std::string> explicitly_requested_users_;
};
}  // namespace Kiran