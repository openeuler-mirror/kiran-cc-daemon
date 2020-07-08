/*
 * @Author       : tangjie02
 * @Date         : 2020-06-18 17:34:09
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-07-02 16:21:49
 * @Description  : 
 * @FilePath     : /kiran-system-daemon/plugins/accounts/accounts-plugin.cpp
 */

#include "plugins/accounts/accounts-plugin.h"

#include <cstdio>

#include "lib/log.h"
#include "plugins/accounts/accounts-manager.h"

PLUGIN_EXPORT_FUNC_DEF(AccountsPlugin);

namespace Kiran
{
AccountsPlugin::AccountsPlugin()
{
}

AccountsPlugin::~AccountsPlugin()
{
}

void AccountsPlugin::activate()
{
    SETTINGS_PROFILE("active accounts plugin.");

    AccountsManager::global_init();
}

}  // namespace Kiran