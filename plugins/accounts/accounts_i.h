/*
 * @Author       : tangjie02
 * @Date         : 2020-11-02 20:37:27
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-11-02 20:49:31
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/plugins/accounts/user_i.h
 */
#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

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