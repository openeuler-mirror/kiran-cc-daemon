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

    /* PEP: Password expiration policy
    chage -l命令显示的参数解释：
    最近一次修改密码时间（last_change_time）：
    密码过期时间：等于last_change_time+max_days_between_changes
    密码失效时间：等于last_change_time+max_days_between_changes+inactive_days_after_expiration
    账户过期时间（expiration_time）：当前时间如果大于过期时间，则账户被锁定
    两次改变密码之间相距的最小天数(min_days_between_changes)
    两次改变密码之间相距的最大天数(max_days_between_changes)
    在密码过期之前警告的天数(days_to_warn)

    正常的顺序应该是：密码修改时间 < 密码过期时间 < 密码失效时间(密码无效) < 账户过期时间（账户被锁定）
    */

#define ACCOUNTS_PEP_EXPIRATION_TIME "expiration_time"               // 账户过期时间。值为从1970-01-01到过期时间的天数。-1表示不设置，如果账户过期则被锁定，需要联系管理员解锁
#define ACCOUNTS_PEP_LAST_CHANGED_TIME "last_change_time"            // 最近一次修改密码的时间。值为从1970-01-01开始到密码修改时间的天数。如果设置为0表示密码必须修改。
#define ACCOUNTS_PEP_MIN_DAYS "min_days_between_changes"             // 密码允许被修改的最小天数，0表示任何时候都可以修改密码
#define ACCOUNTS_PEP_MAX_DAYS "max_days_between_changes"             // 密码保持有效的最大天数，如果密码最近一次修改的时间+密码保持有效的最大天数<当前时间，则密码被置为无效
#define ACCOUNTS_PEP_DAYS_TO_WARN "days_to_warn"                     // 密码过期前多少天开始给予密码过期提示。
#define ACCOUNTS_PEP_INACTIVE_DAYS "inactive_days_after_expiration"  // 密码过期后多少天变为inactive状态。inactive状态下用户不能修改密码，需要联系管理员进行修改。

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