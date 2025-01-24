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

#include "power-upower.h"
#include <QDBusArgument>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusObjectPath>
#include <QDBusVariant>
#include <QVariant>
#include <QVariantMap>
#include "lib/base/base.h"
#include "power-upower-device.h"

namespace Kiran
{
#define UPOWER_DBUS_NAME "org.freedesktop.UPower"
#define UPOWER_DBUS_OBJECT "/org/freedesktop/UPower"
#define UPOWER_DBUS_INTERFACE "org.freedesktop.UPower"
#define UPOWER_DBUS_PROP_ON_BATTERY "OnBattery"
#define UPOWER_DBUS_PROP_LID_IS_CLOSED "LidIsClosed"
#define UPOWER_DBUS_PROP_LID_IS_PRESENT "LidIsPresent"

PowerUPower::PowerUPower() : m_onBattery(false),
                             m_lidIsClosed(false),
                             m_lidIsPresent(false)
{
    m_upowerInterface = new QDBusInterface(UPOWER_DBUS_NAME,
                                           UPOWER_DBUS_OBJECT,
                                           UPOWER_DBUS_INTERFACE,
                                           QDBusConnection::systemBus(),
                                           this);
}

void PowerUPower::init()
{
    m_onBattery = m_upowerInterface->property(UPOWER_DBUS_PROP_ON_BATTERY).toBool();
    m_lidIsClosed = m_upowerInterface->property(UPOWER_DBUS_PROP_LID_IS_CLOSED).toBool();
    m_lidIsPresent = m_upowerInterface->property(UPOWER_DBUS_PROP_LID_IS_PRESENT).toBool();

    auto displayDeviceObjectPath = getDisplayDeviceObjectPath();
    m_displayDevice = QSharedPointer<PowerUPowerDevice>::create(displayDeviceObjectPath);
    connect(m_displayDevice.data(),
            &PowerUPowerDevice::propsChanged,
            std::bind(&PowerUPower::processDevicePropsChanged, this, std::placeholders::_1, std::placeholders::_2, m_displayDevice->getObjectPath()));

    auto devicesObjectPath = getDevicesObjectPath();
    for (auto &o : devicesObjectPath)
    {
        addUPowerDevice(o);
    }

    // TODO:测试第三个参数是否正确？应该是org.freedesktop.DBus.Properties?
    QDBusConnection::systemBus().connect(UPOWER_DBUS_NAME,
                                         UPOWER_DBUS_OBJECT,
                                         UPOWER_DBUS_INTERFACE,
                                         "PropertiesChanged",
                                         this,
                                         SLOT(processPropertiesChanged(const QDBusMessage &)));

    QDBusConnection::systemBus().connect(UPOWER_DBUS_NAME,
                                         UPOWER_DBUS_OBJECT,
                                         UPOWER_DBUS_INTERFACE,
                                         "DeviceAdd",
                                         this,
                                         SLOT(processDeviceAdded(const QDBusMessage &)));

    QDBusConnection::systemBus().connect(UPOWER_DBUS_NAME,
                                         UPOWER_DBUS_OBJECT,
                                         UPOWER_DBUS_INTERFACE,
                                         "DeviceRemoved",
                                         this,
                                         SLOT(processDeviceRemoved(const QDBusMessage &)));
}

QString PowerUPower::getDisplayDeviceObjectPath()
{
    auto sendMessage = QDBusMessage::createMethodCall(UPOWER_DBUS_NAME,
                                                      UPOWER_DBUS_OBJECT,
                                                      UPOWER_DBUS_INTERFACE,
                                                      "GetDisplayDevice");

    auto replyMessage = QDBusConnection::systemBus().call(sendMessage, QDBus::Block);
    if (replyMessage.type() == QDBusMessage::ErrorMessage)
    {
        KLOG_WARNING(power) << "Call GetDisplayDevice return error:" << replyMessage.errorMessage();
        return QString();
    }
    else
    {
        return replyMessage.arguments().takeFirst().value<QDBusObjectPath>().path();
    }
}

QStringList PowerUPower::getDevicesObjectPath()
{
    auto sendMessage = QDBusMessage::createMethodCall(UPOWER_DBUS_NAME,
                                                      UPOWER_DBUS_OBJECT,
                                                      UPOWER_DBUS_INTERFACE,
                                                      "EnumerateDevices");

    auto replyMessage = QDBusConnection::systemBus().call(sendMessage, QDBus::Block);
    if (replyMessage.type() == QDBusMessage::ErrorMessage)
    {
        KLOG_WARNING(power) << "Call EnumerateDevices return error:" << replyMessage.errorMessage();
        return QStringList();
    }
    else
    {
        QStringList retval;
        auto devicesVariant = replyMessage.arguments().takeFirst().value<QVariantList>();
        for (auto deviceVariant : devicesVariant)
        {
            retval.push_back(qvariant_cast<QDBusObjectPath>(deviceVariant).path());
        }
        return retval;
    }
}

bool PowerUPower::addUPowerDevice(const QString &objectPath)
{
    auto device = QSharedPointer<PowerUPowerDevice>::create(objectPath);
    if (m_devices.contains(objectPath))
    {
        KLOG_WARNING(power) << "The upwer device" << objectPath << "already exists.";
        return false;
    }
    else
    {
        m_devices.insert(objectPath, device);
    }

    connect(device.data(),
            &PowerUPowerDevice::propsChanged,
            std::bind(&PowerUPower::processDevicePropsChanged, this, std::placeholders::_1, std::placeholders::_2, objectPath));
    return true;
}

bool PowerUPower::delUPowerDevice(const QString &objectPath)
{
    auto iter = m_devices.find(objectPath);
    if (iter == m_devices.end())
    {
        KLOG_WARNING(power) << "The upower device" << objectPath << "doesn't exist.";
        return false;
    }
    m_devices.erase(iter);
    return true;
}

void PowerUPower::processDevicePropsChanged(const UPowerDeviceProps &oldProps,
                                            const UPowerDeviceProps &newProps,
                                            const QString &deviceObjectPath)
{
    // 不处理单个电池设备的状态信息，电池设备的状态信息以混合设备的为准
    if (newProps.type == UP_DEVICE_KIND_BATTERY &&
        deviceObjectPath != m_displayDevice->getObjectPath())
    {
        return;
    }

    QSharedPointer<PowerUPowerDevice> device;
    if (deviceObjectPath == m_displayDevice->getObjectPath())
    {
        device = m_displayDevice;
    }
    else
    {
        device = getDevice(deviceObjectPath);
    }
    RETURN_IF_FALSE(device);

    if (oldProps.state != newProps.state)
    {
        switch (newProps.state)
        {
        case UP_DEVICE_STATE_CHARGING:
            Q_EMIT deviceStatusChanged(device, UPowerDeviceEvent::UPOWER_DEVICE_EVENT_CHARGING);
            break;
        case UP_DEVICE_STATE_DISCHARGING:
            Q_EMIT deviceStatusChanged(device, UPowerDeviceEvent::UPOWER_DEVICE_EVENT_DISCHARGING);
            break;
        case UP_DEVICE_STATE_FULLY_CHARGED:
            Q_EMIT deviceStatusChanged(device, UPowerDeviceEvent::UPOWER_DEVICE_EVENT_FULLY_CHARGED);
            break;
        default:
            break;
        }
    }

    if (oldProps.warningLevel != newProps.warningLevel)
    {
        switch (newProps.warningLevel)
        {
        case UP_DEVICE_LEVEL_LOW:
            Q_EMIT deviceStatusChanged(device, UPowerDeviceEvent::UPOWER_DEVICE_EVENT_CHARGE_LOW);
            break;
        case UP_DEVICE_LEVEL_CRITICAL:
            Q_EMIT deviceStatusChanged(device, UPowerDeviceEvent::UPOWER_DEVICE_EVENT_CHARGE_CRITICAL);
            break;
        case UP_DEVICE_LEVEL_ACTION:
            Q_EMIT deviceStatusChanged(device, UPowerDeviceEvent::UPOWER_DEVICE_EVENT_CHARGE_ACTION);
        default:
            break;
        }
    }

    Q_EMIT devicePropsChanged(device, oldProps, newProps);
}

void PowerUPower::processPropertiesChanged(const QDBusMessage &message)
{
    QList<QVariant> args = message.arguments();
    RETURN_IF_TRUE(args.count() != 3);

    QVariantMap changedProperties = qdbus_cast<QVariantMap>(args.at(1).value<QDBusArgument>());
    for (auto iter = changedProperties.begin(); iter != changedProperties.end(); iter++)
    {
        switch (shash(iter.key().toLatin1().data()))
        {
        case CONNECT(UPOWER_DBUS_PROP_ON_BATTERY, _hash):
            m_onBattery = iter.value().toBool();
            Q_EMIT onBatteryChanged(m_onBattery);
            break;
        case CONNECT(UPOWER_DBUS_PROP_LID_IS_CLOSED, _hash):
            m_lidIsClosed = iter.value().toBool();
            Q_EMIT lidIsClosedChanged(m_lidIsClosed);
            break;
        case CONNECT(UPOWER_DBUS_PROP_LID_IS_PRESENT, _hash):
            m_lidIsPresent = iter.value().toBool();
            Q_EMIT lidIsPresentChanged(m_lidIsPresent);
            break;
        default:
            break;
        }
    }
}

void PowerUPower::processDeviceAdded(const QDBusMessage &message)
{
    auto objectPath = message.arguments().takeFirst().value<QDBusObjectPath>().path();
    addUPowerDevice(objectPath);
}
void PowerUPower::processDeviceRemoved(const QDBusMessage &message)
{
    auto objectPath = message.arguments().takeFirst().value<QDBusObjectPath>().path();
    delUPowerDevice(objectPath);
}

}  // namespace Kiran