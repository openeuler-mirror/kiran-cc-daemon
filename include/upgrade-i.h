/**
 * Copyright (c) 2020 ~ 2025 KylinSec Co., Ltd.
 * kiran-cc-daemon is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     yuanxing <yuanxing@kylinsec.com.cn>
 */

#pragma once

#include <QFlags>

#ifdef __cplusplus
extern "C"
{
#endif

#define UPGRADE_DBUS_NAME "com.kylinsec.Kiran.SystemDaemon.Upgrade"
#define UPGRADE_OBJECT_PATH "/com/kylinsec/Kiran/SystemDaemon/Upgrade"
#define UPGRADE_DBUS_INTERFACE_NAME "com.kylinsec.Kiran.SystemDaemon.Upgrade"

#define DEFAULT_REMINDER_INTERVAL 7             // 默认每周提醒
#define DEFAULT_CACHE_UPDATE_INTERVAL_HOURS 24  // 默认24小时更新一次缓存
#define DEFAULT_DATE_TIME_FORMAT "yyyy-MM-dd HH:mm:ss"

    enum ReminderInterval
    {
        REMINDER_INTERVAL_NEVER = 0,       // 不提醒
        REMINDER_INTERVAL_WEEKLY = 7,      // 每周提醒
        REMINDER_INTERVAL_MONTHLY = 30,    // 每月提醒
        REMINDER_INTERVAL_QUARTERLY = 90,  // 每季度提醒
        REMINDER_INTERVAL_LAST
    };

    /**
    * @brief Advisory 类型标志枚举
    */
    enum AdvisoryKindFlag
    {
        ADVISORY_KIND_UNKNOWN = 0,
        ADVISORY_KIND_SECURITY = 1 << 0,     // 安全更新
        ADVISORY_KIND_BUGFIX = 1 << 1,       // Bug修复
        ADVISORY_KIND_ENHANCEMENT = 1 << 2,  // 功能增强
        ADVISORY_KIND_NEWPACKAGE = 1 << 3    // 新包
    };

#ifdef __cplusplus
}

Q_DECLARE_FLAGS(AdvisoryKindFlags, AdvisoryKindFlag)
Q_DECLARE_OPERATORS_FOR_FLAGS(AdvisoryKindFlags)
#endif
