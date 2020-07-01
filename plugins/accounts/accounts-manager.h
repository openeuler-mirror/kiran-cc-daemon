/*
 * @Author       : tangjie02
 * @Date         : 2020-06-19 10:08:59
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-07-01 11:08:38
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
    virtual ~AccountsManager(){};

    static AccountsManager *get_instance() { return instance_; };

    static void global_init();

    static void global_deinit() { delete instance_; };

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

    void add_user(ActUser *act_user);
    void del_user(ActUser *act_user);

private:
    static AccountsManager *instance_;

    ActUserManager *act_user_manager_;

    std::map<std::string, std::shared_ptr<User>> users_;
};
}  // namespace Kiran