/*
 * @Author       : tangjie02
 * @Date         : 2020-06-18 17:34:09
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-09-02 15:10:19
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/plugins/accounts/accounts-plugin.cpp
 */

#include "plugins/accounts/accounts-plugin.h"

#include <cstdio>

#include "lib/base/log.h"
#include "plugins/accounts/accounts-manager.h"
#include "plugins/accounts/accounts-wrapper.h"

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

    AccountsWrapper::global_init();
    AccountsManager::global_init(AccountsWrapper::get_instance());
}

void AccountsPlugin::deactivate()
{
    SETTINGS_PROFILE("deactive accounts plugin.");

    AccountsManager::global_deinit();
    AccountsWrapper::global_deinit();
}

}  // namespace Kiran