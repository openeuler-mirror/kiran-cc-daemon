/*
 * @Author       : tangjie02
 * @Date         : 2020-06-18 17:34:09
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-06-30 20:32:01
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

    Log::global_init();
    AccountsManager::global_init();
}

void AccountsPlugin::register_object(const Glib::RefPtr<Gio::DBus::Connection> &connection)
{
}

}  // namespace Kiran