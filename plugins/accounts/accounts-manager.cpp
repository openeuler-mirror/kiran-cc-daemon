/*
 * @Author       : tangjie02
 * @Date         : 2020-06-19 10:09:05
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-06-30 15:10:47
 * @Description  : 
 * @FilePath     : /kiran-system-daemon/plugins/accounts/accounts-manager.cpp
 */

#include "plugins/accounts/accounts-manager.h"

namespace Kiran
{
AccountsManager::AccountsManager() : accounts_dbus_(new AccountsDBus())
{
}

AccountsManager* AccountsManager::instance_ = nullptr;
void AccountsManager::global_init()
{
    instance_ = new AccountsManager();
    instance_->init();
}

void AccountsManager::init()
{
    this->accounts_dbus_->init();
}
}  // namespace Kiran
