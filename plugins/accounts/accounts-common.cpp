/*
 * @Author       : tangjie02
 * @Date         : 2020-07-01 09:30:13
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-07-03 15:38:08
 * @Description  : 
 * @FilePath     : /kiran-system-daemon/plugins/accounts/accounts-common.cpp
 */

#include "plugins/accounts/accounts-common.h"

namespace Kiran
{
static const GDBusErrorEntry accounts_error_entries[] =
    {{static_cast<int32_t>(AccountsError::ERROR_FAILED), "com.unikylin.Kiran.Accounts.Error.Failed"},
     {static_cast<int32_t>(AccountsError::ERROR_USER_EXISTS), "com.unikylin.Kiran.Accounts.Error.UserExists"},
     {static_cast<int32_t>(AccountsError::ERROR_USER_DOES_NOT_EXIST), "com.unikylin.Kiran.Accounts.Error.UserDoesNotExist"},
     {static_cast<int32_t>(AccountsError::ERROR_PERMISSION_DENIED), "com.unikylin.Kiran.Accounts.Error.PermissionDenied"},
     {static_cast<int32_t>(AccountsError::ERROR_NOT_SUPPORTED), "com.unikylin.Kiran.Accounts.Error.NotSupported"}};

GQuark error_quark(void)
{
    static volatile gsize quark_volatile = 0;

    if (!quark_volatile)
    {
        g_dbus_error_register_error_domain("accounts_error",
                                           &quark_volatile,
                                           accounts_error_entries,
                                           G_N_ELEMENTS(accounts_error_entries));
    }

    return (GQuark)quark_volatile;
}
}  // namespace Kiran