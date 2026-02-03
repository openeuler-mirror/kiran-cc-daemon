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

#include <QDBusArgument>
#include <QFlags>
#include <QString>
#include <QStringList>

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

    // 后台状态枚举
    enum BackendStatus
    {
        BACKEND_STATUS_IDLE = 0,      // 空闲
        BACKEND_STATUS_SCANNING,      // 正在扫描
        BACKEND_STATUS_SOLVING_DEPS,  // 正在解析依赖
        BACKEND_STATUS_UPGRADING,     // 正在更新
    };

    // 提醒间隔枚举
    enum ReminderInterval
    {
        REMINDER_INTERVAL_NEVER = 0,       // 不提醒
        REMINDER_INTERVAL_WEEKLY = 7,      // 每周提醒
        REMINDER_INTERVAL_MONTHLY = 30,    // 每月提醒
        REMINDER_INTERVAL_QUARTERLY = 90,  // 每季度提醒
        REMINDER_INTERVAL_LAST
    };

    //类型标志枚举
    enum AdvisoryKindFlag
    {
        ADVISORY_KIND_NONE = 0,              // 无
        ADVISORY_KIND_UNKNOWN = 1 << 0,      // 未知
        ADVISORY_KIND_SECURITY = 1 << 1,     // 安全更新
        ADVISORY_KIND_BUGFIX = 1 << 2,       // Bug修复
        ADVISORY_KIND_ENHANCEMENT = 1 << 3,  // 功能增强
        ADVISORY_KIND_NEWPACKAGE = 1 << 4,   // 新包
    };

    // 升级结果枚举
    enum UpgradeResult
    {
        UPGRADE_RESULT_UNKNOWN = 0,  // 未知（升级开始时的初始状态）
        UPGRADE_RESULT_SUCCESS = 1,  // 成功
        UPGRADE_RESULT_FAILED = 2    // 失败
    };

    // 升级历史记录结构体，用于存储升级过程中的完整信息
    struct UpgradeHistory
    {
        QString upgradeTime;          // 升级时间（格式：yyyy-MM-dd HH:mm:ss）
        UpgradeResult result;         // 升级结果（枚举值）
        QString errorMessage;         // 错误信息
        QStringList successPackages;  // 成功升级的包列表
        QStringList failedPackages;   // 失败的包列表

        UpgradeHistory()
            : upgradeTime(""), result(UPGRADE_RESULT_UNKNOWN), errorMessage(""), successPackages(), failedPackages()
        {
        }

        UpgradeHistory(const QString &upgradeTime, UpgradeResult result, const QString &errMsg,
                       const QStringList &successPkgs, const QStringList &failedPkgs)
            : upgradeTime(upgradeTime),
              result(result),
              errorMessage(errMsg),
              successPackages(successPkgs),
              failedPackages(failedPkgs)
        {
        }

        friend QDBusArgument &operator<<(QDBusArgument &argument, const UpgradeHistory &history)
        {
            argument.beginStructure();
            argument << history.upgradeTime
                     << static_cast<int>(history.result)
                     << history.errorMessage
                     << history.successPackages
                     << history.failedPackages;
            argument.endStructure();
            return argument;
        }

        friend const QDBusArgument &operator>>(const QDBusArgument &argument, UpgradeHistory &history)
        {
            argument.beginStructure();
            int result;
            argument >> history.upgradeTime >> result >> history.errorMessage >> history.successPackages >> history.failedPackages;
            history.result = static_cast<UpgradeResult>(result);
            argument.endStructure();
            return argument;
        }
    };

#ifdef __cplusplus
}

Q_DECLARE_FLAGS(AdvisoryKindFlags, AdvisoryKindFlag)
Q_DECLARE_OPERATORS_FOR_FLAGS(AdvisoryKindFlags)
Q_DECLARE_METATYPE(UpgradeHistory)
#endif
