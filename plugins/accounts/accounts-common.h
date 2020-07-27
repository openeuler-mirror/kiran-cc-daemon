/*
 * @Author       : tangjie02
 * @Date         : 2020-07-01 09:30:07
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-07-27 15:26:12
 * @Description  : 
 * @FilePath     : /kiran-system-daemon/plugins/accounts/accounts-common.h
 */
#pragma once

#include <gio/gio.h>

namespace Kiran
{
#define ACCOUNTS_DBUS_NAME "com.unikylin.Kiran.SystemDaemon.Accounts"
enum class AccountsError : int32_t
{
    ERROR_FAILED,
    ERROR_USER_EXISTS,
    ERROR_USER_DOES_NOT_EXIST,
    ERROR_PERMISSION_DENIED,
    ERROR_NOT_SUPPORTED,
    NUM_ERRORS
};

#define ACCOUNTS_ERROR error_quark()

GQuark error_quark(void);

#define USERDIR "/var/lib/AccountsService/users"
#define ICONDIR "/var/lib/AccountsService/icons"
#define PATH_GDM_CUSTOM "/etc/gdm/custom.conf"
#define ADMIN_GROUP "wheel"

#define AUTH_USER_ADMIN "com.unikylin.kiran.system-daemon.accounts.user-administration"
#define AUTH_CHANGE_OWN_USER_DATA "com.unikylin.kiran.system-daemon.accounts.change-own-user-data"
#define AUTH_CHANGE_OWN_PASSWORD "com.unikylin.kiran.system-daemon.accounts.change-own-password"

}  // namespace Kiran