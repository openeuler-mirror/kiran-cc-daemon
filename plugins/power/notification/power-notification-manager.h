/**
 * Copyright (c) 2020 ~ 2021 KylinSec Co., Ltd.
 * kiran-cc-daemon is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
 */

#pragma once

#include <QObject>
#include <QSharedPointer>

typedef struct _NotifyNotification NotifyNotification;
class QGSettings;

namespace Kiran
{
class PowerWrapperManager;
class PowerUPower;
class PowerUPowerDevice;

class PowerNotificationManager : public QObject
{
    Q_OBJECT

public:
    PowerNotificationManager(PowerWrapperManager* wrapperManager);
    virtual ~PowerNotificationManager();

    static PowerNotificationManager* getInstance() { return m_instance; };

    static void globalInit(PowerWrapperManager* wrapperManager);

    static void globalDeinit() { delete m_instance; };

private:
    void init();

    bool messageNotify(const QString& title,
                       const QString& message,
                       uint32_t timeout,
                       const QString& iconName,
                       int urgency);

    void processDeviceStatusChanged(QSharedPointer<PowerUPowerDevice> device, int event);
    void processDeviceDischarging(QSharedPointer<PowerUPowerDevice> device);
    void processDeviceFullyCharged(QSharedPointer<PowerUPowerDevice> device);
    void processDeviceChargeLow(QSharedPointer<PowerUPowerDevice> device);
    void processDeviceChargeCritical(QSharedPointer<PowerUPowerDevice> device);
    void processDeviceChargeAction(QSharedPointer<PowerUPowerDevice> device);

private:
    static PowerNotificationManager* m_instance;

    PowerWrapperManager* m_wrapperManager;
    QSharedPointer<PowerUPower> m_upowerClient;
    NotifyNotification* m_deviceNotification;
    QGSettings* m_powerSettings;
};
}  // namespace Kiran