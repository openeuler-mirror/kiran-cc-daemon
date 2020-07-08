/*
 * @Author       : tangjie02
 * @Date         : 2020-06-19 10:08:59
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-07-08 09:29:37
 * @Description  : 
 * @FilePath     : /kiran-system-daemon/plugins/accounts/accounts-manager.h
 */

#include <accounts_dbus_stub.h>

#include "plugins/accounts/user.h"

namespace Kiran
{
class AccountsManager : public System::AccountsStub
{
public:
    AccountsManager();
    virtual ~AccountsManager();

    static AccountsManager *get_instance() { return instance_; };

    static void global_init();

    static void global_deinit() { delete instance_; };

    std::shared_ptr<User> lookup_user(ActUser *act_user);
    std::shared_ptr<User> lookup_user_by_name(const std::string &user_name);
    std::shared_ptr<User> lookup_user_by_uid(int64_t uid);

protected:
    virtual void CreateUser(const Glib::ustring &name,
                            const Glib::ustring &real_name,
                            gint32 account_type,
                            MethodInvocation &invocation);

    virtual void DeleteUser(gint64 id,
                            bool remove_files,
                            MethodInvocation &invocation);

    virtual void FindUserById(gint64 id,
                              MethodInvocation &invocation);

    virtual void FindUserByName(const Glib::ustring &name,
                                MethodInvocation &invocation);

    static void signal_user_added(ActUser *act_user, gpointer user_data);
    static void signal_user_removed(ActUser *act_user, gpointer user_data);

private:
    void init();

    void load_users();

    std::shared_ptr<User> add_user(ActUser *act_user);
    void del_user(ActUser *act_user);

    void on_bus_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name);
    void on_name_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name);
    void on_name_lost(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name);

private:
    static AccountsManager *instance_;

    uint32_t dbus_connect_id_;

    uint32_t object_register_id_;

    ActUserManager *act_user_manager_;

    std::map<std::string, std::shared_ptr<User>> users_;
    std::map<int64_t, std::weak_ptr<User>> users_by_uid_;
};
}  // namespace Kiran