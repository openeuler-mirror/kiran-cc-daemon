/*
 * @Author       : tangjie02
 * @Date         : 2020-06-19 10:08:59
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-06-30 15:10:30
 * @Description  : 
 * @FilePath     : /kiran-system-daemon/plugins/accounts/accounts-manager.h
 */

#include "plugins/accounts/accounts-dbus.h"

namespace Kiran
{
class AccountsManager
{
public:
    AccountsManager();
    virtual ~AccountsManager(){};

    static AccountsManager* get_instance() { return instance_; };

    static void global_init();

    static void global_deinit() { delete instance_; };

private:
    void init();

private:
    static AccountsManager* instance_;

    std::shared_ptr<AccountsDBus> accounts_dbus_;
};
}  // namespace Kiran