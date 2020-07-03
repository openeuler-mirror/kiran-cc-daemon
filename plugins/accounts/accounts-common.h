/*
 * @Author       : tangjie02
 * @Date         : 2020-07-01 09:30:07
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-07-01 10:10:04
 * @Description  : 
 * @FilePath     : /kiran-system-daemon/plugins/accounts/accounts-common.h
 */

#include <gio/gio.h>

namespace Kiran
{
#define ACCOUNTS_DBUS_NAME "com.unikylin.Kiran.System.Accounts"
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

}  // namespace Kiran