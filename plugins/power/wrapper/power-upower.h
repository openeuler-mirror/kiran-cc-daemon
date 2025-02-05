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

#include <QMap>
#include <QObject>
#include <QSharedPointer>
#include <QStringList>
#include <QVector>

class QDBusInterface;
class QDBusMessage;

namespace Kiran
{

class PowerUPowerDevice;
struct UPowerDeviceProps;

class PowerUPower : public QObject
{
    Q_OBJECT

public:
    PowerUPower();
    virtual ~PowerUPower(){};

    void init();

    // 是否使用电池供电
    bool getOnBattery() { return m_onBattery; };
    // 笔记本盖子是否关闭
    bool getLidIsClosed() { return m_lidIsClosed; };
    // 是否有笔记本盖子
    bool getLidIsPresent() { return m_lidIsPresent; };

    /* 混合电池设备。部分机器可能存在多个电池设备，当使用电池供电时，系统供电状态应该依赖所有电池设备的电量情况，而不是单个电池的电量，
       因此，如果想获取系统电池整体使用情况，需要使用混合电源设备*/
    QSharedPointer<PowerUPowerDevice> getDisplayDevice() { return m_displayDevice; };
    QList<QSharedPointer<PowerUPowerDevice>> getDevices() { return m_devices.values(); };
    QSharedPointer<PowerUPowerDevice> getDevice(const QString &objectPath) { return m_devices.value(objectPath); };

Q_SIGNALS:
    void onBatteryChanged(bool onBattery);
    void lidIsClosedChanged(bool lidIsClosed);
    void lidIsPresentChanged(bool lidIsPresent);
    // event: UPowerDeviceEvent
    void
    deviceStatusChanged(QSharedPointer<PowerUPowerDevice> device, int event);
    void devicePropsChanged(QSharedPointer<PowerUPowerDevice> device, const UPowerDeviceProps &oldProps, const UPowerDeviceProps &newProps);

private:
    QString getDisplayDeviceObjectPath();
    QStringList getDevicesObjectPath();

    bool addUPowerDevice(const QString &objectPath);
    bool delUPowerDevice(const QString &objectPath);

    void processDevicePropsChanged(const UPowerDeviceProps &oldProps,
                                   const UPowerDeviceProps &newProps,
                                   const QString &deviceObjectPath);

private Q_SLOTS:
    void processPropertiesChanged(const QDBusMessage &message);
    void processDeviceAdded(const QDBusMessage &message);
    void processDeviceRemoved(const QDBusMessage &message);

private:
    QDBusInterface *m_upowerInterface;

    bool m_onBattery;
    bool m_lidIsClosed;
    bool m_lidIsPresent;

    QSharedPointer<PowerUPowerDevice> m_displayDevice;
    QMap<QString, QSharedPointer<PowerUPowerDevice>> m_devices;
};
}  // namespace Kiran