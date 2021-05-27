/**
 * @file          /kiran-cc-daemon/include/accounts_i.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#pragma once

#ifndef ACCOUNTS_NEW_INTERFACE
#warning This file will be deprecated. please use accounts-i.h file
#endif

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
        // polkit管理员账户
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

    enum AccountsAuthMode
    {
        // 没有任何验证方式
        ACCOUNTS_AUTH_MODE_NONE = 0,
        // 密码验证
        ACCOUNTS_AUTH_MODE_PASSWORD = (1 << 0),
        // 指纹验证
        ACCOUNTS_AUTH_MODE_FINGERPRINT = (1 << 1),
        // 人脸识别验证
        ACCOUNTS_AUTH_MODE_FACE = (1 << 2),
        ACCOUNTS_AUTH_MODE_LAST = (1 << 3),
    };

#ifdef __cplusplus
}
#endif