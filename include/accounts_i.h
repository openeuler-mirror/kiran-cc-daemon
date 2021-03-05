/**
 * @file          /kiran-cc-daemon/include/accounts_i.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#define ACCOUNTS_DBUS_NAME "com.kylinsec.Kiran.SystemDaemon.Accounts"
#define ACCOUNTS_OBJECT_PATH "/com/kylinsec/Kiran/SystemDaemon/Accounts"
#define ACCOUNTS_DBUS_INTERFACE_NAME "com.kylinsec.Kiran.SystemDaemon.Accounts"

    enum AccountsAccountType
    {
        // 普通账户
        ACCOUNTS_ACCOUNT_TYPE_STANDARD,
        // 管理员账户
        ACCOUNTS_ACCOUNT_TYPE_ADMINISTRATOR,
        ACCOUNTS_ACCOUNT_TYPE_LAST
    };

    enum AccountsPasswordMode
    {
        // 正常情况
        ACCOUNTS_PASSWORD_MODE_REGULAR,
        // 登陆时需要设置密码
        ACCOUNTS_PASSWORD_MODE_SET_AT_LOGIN,
        // 设置为无密码登陆
        ACCOUNTS_PASSWORD_MODE_NONE,
        ACCOUNTS_PASSWORD_MODE_LAST
    };

#ifdef __cplusplus
}
#endif