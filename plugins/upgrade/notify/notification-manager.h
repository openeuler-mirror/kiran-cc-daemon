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

#include <QDateTime>
#include <QObject>
#include <QSharedPointer>
#include <QTimer>

struct _NotifyNotification;
typedef struct _NotifyNotification NotifyNotification;

class KSDUpgradeProxy;
namespace Kiran
{
class NotificationManager : public QObject
{
    Q_OBJECT

public:
    NotificationManager();
    virtual ~NotificationManager();

private slots:
    void reminder();
    void handleDBusPropertyChanged(const QString &name, const QVariant &value);
    void handleScanResult(bool success, const QString &errorMessage);

private:
    void init();
    KSDUpgradeProxy *getUpgradeProxy();
    int getVaildIntval();
    void sendNotification();

    /**
     * @brief 安排下次提醒（用于初始化和配置变更）
     */
    void scheduleNextReminder();
    /**
     * @brief 设置下次提醒时间（统一入口）
     * @param intervalDays 提醒间隔（天数），1表示24小时后，0表示从不提醒
     */
    void setNextReminder(int intervalDays);
    /**
     * @brief 启动定时器
     * @param nextTime 下次提醒的绝对时间
     */
    void startTimerForNextReminder(const QDateTime &nextTime);
    /**
     * @brief 计算并设置下次提醒时间
     * @param intervalDays 提醒间隔（天数），0表示从不提醒
     * @param baseTime 计算基准时间
     */
    void calculateAndSetNextReminder(int intervalDays, const QDateTime &baseTime);

private:
    QSharedPointer<NotifyNotification> m_notification;
    QTimer *m_timer;
    QDateTime m_latestReminderTime;
    QDateTime m_nextReminderTime;
    KSDUpgradeProxy *m_upgradeProxy;
    bool m_hasUpgrades;
    bool m_scanInProgress;
};
}  // namespace Kiran