/**
 * @Copyright (C) 2020 ~ 2021 KylinSec Co., Ltd. 
 *
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; If not, see <http: //www.gnu.org/licenses/>. 
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