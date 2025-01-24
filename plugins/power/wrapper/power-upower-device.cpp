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

#include "power-upower-device.h"
#include <QDBusArgument>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusVariant>
#include <QVariantMap>
#include "lib/base/base.h"

namespace Kiran
{
#define UPOWER_DEVICE_DBUS_NAME "org.freedesktop.UPower"
#define UPOWER_DEVICE_DBUS_INTERFACE "org.freedesktop.UPower.Device"

#define UPOWER_DEVICE_DBUS_PROP_NATIVE_PATH "NativePath"
#define UPOWER_DEVICE_DBUS_PROP_VENDOR "Vendor"
#define UPOWER_DEVICE_DBUS_PROP_MODEL "Model"
#define UPOWER_DEVICE_DBUS_PROP_SERIAL "Serial"
#define UPOWER_DEVICE_DBUS_PROP_UPDATE_TIME "UpdateTime"
#define UPOWER_DEVICE_DBUS_PROP_TYPE "Type"
#define UPOWER_DEVICE_DBUS_PROP_POWER_SUPPLY "PowerSupply"
#define UPOWER_DEVICE_DBUS_PROP_HAS_HISTORY "HasHistory"
#define UPOWER_DEVICE_DBUS_PROP_HAS_STATISTICS "HasStatistics"
#define UPOWER_DEVICE_DBUS_PROP_ONLINE "Online"
#define UPOWER_DEVICE_DBUS_PROP_ENERGY "Energy"
#define UPOWER_DEVICE_DBUS_PROP_ENERGY_EMPTY "EnergyEmpty"
#define UPOWER_DEVICE_DBUS_PROP_ENERGY_FULL "EnergyFull"
#define UPOWER_DEVICE_DBUS_PROP_ENERGY_FULL_DESIGN "EnergyFullDesign"
#define UPOWER_DEVICE_DBUS_PROP_ENERGY_RATE "EnergyRate"
#define UPOWER_DEVICE_DBUS_PROP_VOLTAGE "Voltage"
#define UPOWER_DEVICE_DBUS_PROP_LUMINOSITY "Luminosity"
#define UPOWER_DEVICE_DBUS_PROP_TIME_TO_EMPTY "TimeToEmpty"
#define UPOWER_DEVICE_DBUS_PROP_TIME_TO_FULL "TimeToFull"
#define UPOWER_DEVICE_DBUS_PROP_PERCENTAGE "Percentage"
#define UPOWER_DEVICE_DBUS_PROP_TEMPERATURE "Temperature"
#define UPOWER_DEVICE_DBUS_PROP_IS_PRESENT "IsPresent"
#define UPOWER_DEVICE_DBUS_PROP_STATE "State"
#define UPOWER_DEVICE_DBUS_PROP_IS_RECHARGEABLE "IsRechargeable"
#define UPOWER_DEVICE_DBUS_PROP_CAPACITY "Capacity"
#define UPOWER_DEVICE_DBUS_PROP_TECHNOLOGY "Technology"
#define UPOWER_DEVICE_DBUS_PROP_WARNING_LEVEL "WarningLevel"
#define UPOWER_DEVICE_DBUS_PROP_BATTERY_LEVEL "BatteryLevel"
#define UPOWER_DEVICE_DBUS_PROP_ICON_NAME "IconName"

PowerUPowerDevice::PowerUPowerDevice(const QString& objectPath) : m_objectPath(objectPath)
{
    m_upowerDeviceInterface = new QDBusInterface(UPOWER_DEVICE_DBUS_NAME,
                                                 m_objectPath,
                                                 UPOWER_DEVICE_DBUS_INTERFACE,
                                                 QDBusConnection::systemBus(),
                                                 this);

    loadDeviceProps();
    // TODO:测试第三个参数是否正确？应该是org.freedesktop.DBus.Properties?
    QDBusConnection::systemBus().connect(UPOWER_DEVICE_DBUS_NAME,
                                         m_objectPath,
                                         UPOWER_DEVICE_DBUS_INTERFACE,
                                         "PropertiesChanged",
                                         this,
                                         SLOT(processPropertiesChanged(const QDBusMessage&)));
}

void PowerUPowerDevice::loadDeviceProps()
{
    m_props.nativePath = m_upowerDeviceInterface->property(UPOWER_DEVICE_DBUS_PROP_NATIVE_PATH).toString();
    m_props.vendor = m_upowerDeviceInterface->property(UPOWER_DEVICE_DBUS_PROP_VENDOR).toString();
    m_props.model = m_upowerDeviceInterface->property(UPOWER_DEVICE_DBUS_PROP_MODEL).toString();
    m_props.serial = m_upowerDeviceInterface->property(UPOWER_DEVICE_DBUS_PROP_SERIAL).toString();
    m_props.updateTime = m_upowerDeviceInterface->property(UPOWER_DEVICE_DBUS_PROP_UPDATE_TIME).toULongLong();
    m_props.type = m_upowerDeviceInterface->property(UPOWER_DEVICE_DBUS_PROP_TYPE).toUInt();
    m_props.powerSupply = m_upowerDeviceInterface->property(UPOWER_DEVICE_DBUS_PROP_POWER_SUPPLY).toBool();
    m_props.hasHistory = m_upowerDeviceInterface->property(UPOWER_DEVICE_DBUS_PROP_HAS_HISTORY).toBool();
    m_props.hasStatistics = m_upowerDeviceInterface->property(UPOWER_DEVICE_DBUS_PROP_HAS_STATISTICS).toBool();
    m_props.online = m_upowerDeviceInterface->property(UPOWER_DEVICE_DBUS_PROP_ONLINE).toBool();
    m_props.energy = m_upowerDeviceInterface->property(UPOWER_DEVICE_DBUS_PROP_ENERGY).toDouble();
    m_props.energyEmpty = m_upowerDeviceInterface->property(UPOWER_DEVICE_DBUS_PROP_ENERGY_EMPTY).toDouble();
    m_props.energyFull = m_upowerDeviceInterface->property(UPOWER_DEVICE_DBUS_PROP_ENERGY_FULL).toDouble();
    m_props.energyFullDesign = m_upowerDeviceInterface->property(UPOWER_DEVICE_DBUS_PROP_ENERGY_FULL_DESIGN).toDouble();
    m_props.energyRate = m_upowerDeviceInterface->property(UPOWER_DEVICE_DBUS_PROP_ENERGY_RATE).toDouble();
    m_props.voltage = m_upowerDeviceInterface->property(UPOWER_DEVICE_DBUS_PROP_VOLTAGE).toDouble();
    m_props.luminosity = m_upowerDeviceInterface->property(UPOWER_DEVICE_DBUS_PROP_LUMINOSITY).toDouble();
    m_props.timeToEmpty = m_upowerDeviceInterface->property(UPOWER_DEVICE_DBUS_PROP_TIME_TO_EMPTY).toLongLong();
    m_props.timeToFull = m_upowerDeviceInterface->property(UPOWER_DEVICE_DBUS_PROP_TIME_TO_FULL).toLongLong();
    m_props.percentage = m_upowerDeviceInterface->property(UPOWER_DEVICE_DBUS_PROP_PERCENTAGE).toDouble();
    m_props.temperature = m_upowerDeviceInterface->property(UPOWER_DEVICE_DBUS_PROP_TEMPERATURE).toDouble();
    m_props.isPresent = m_upowerDeviceInterface->property(UPOWER_DEVICE_DBUS_PROP_IS_PRESENT).toBool();
    m_props.state = m_upowerDeviceInterface->property(UPOWER_DEVICE_DBUS_PROP_STATE).toUInt();
    m_props.isRechargeable = m_upowerDeviceInterface->property(UPOWER_DEVICE_DBUS_PROP_IS_RECHARGEABLE).toBool();
    m_props.capacity = m_upowerDeviceInterface->property(UPOWER_DEVICE_DBUS_PROP_CAPACITY).toDouble();
    m_props.technology = m_upowerDeviceInterface->property(UPOWER_DEVICE_DBUS_PROP_TECHNOLOGY).toUInt();
    m_props.warningLevel = m_upowerDeviceInterface->property(UPOWER_DEVICE_DBUS_PROP_WARNING_LEVEL).toUInt();
    m_props.batteryLevel = m_upowerDeviceInterface->property(UPOWER_DEVICE_DBUS_PROP_BATTERY_LEVEL).toUInt();
    m_props.iconName = m_upowerDeviceInterface->property(UPOWER_DEVICE_DBUS_PROP_ICON_NAME).toString();
}

QString PowerUPowerDevice::kind2str(UpDeviceKind typeEnum)
{
    switch (typeEnum)
    {
    case UP_DEVICE_KIND_LINE_POWER:
        return "line-power";
    case UP_DEVICE_KIND_BATTERY:
        return "battery";
    case UP_DEVICE_KIND_UPS:
        return "ups";
    case UP_DEVICE_KIND_MONITOR:
        return "monitor";
    case UP_DEVICE_KIND_MOUSE:
        return "mouse";
    case UP_DEVICE_KIND_KEYBOARD:
        return "keyboard";
    case UP_DEVICE_KIND_PDA:
        return "pda";
    case UP_DEVICE_KIND_PHONE:
        return "phone";
    case UP_DEVICE_KIND_MEDIA_PLAYER:
        return "media-player";
    case UP_DEVICE_KIND_TABLET:
        return "tablet";
    case UP_DEVICE_KIND_COMPUTER:
        return "computer";
    case UP_DEVICE_KIND_GAMING_INPUT:
        return "gaming-input";
    default:
        return "unknown";
    }
    return "unknown";
}

void PowerUPowerDevice::processPropertiesChanged(const QDBusMessage& message)
{
    QList<QVariant> args = message.arguments();
    RETURN_IF_TRUE(args.count() != 3);

    auto oldProps = m_props;

    QVariantMap changedProperties = qdbus_cast<QVariantMap>(args.at(1).value<QDBusArgument>());
    for (auto iter = changedProperties.begin(); iter != changedProperties.end(); iter++)
    {
        switch (shash(iter.key().toLatin1().data()))
        {
        case CONNECT(UPOWER_DEVICE_DBUS_PROP_NATIVE_PATH, _hash):
            m_props.nativePath = iter.value().toString();
            break;
        case CONNECT(UPOWER_DEVICE_DBUS_PROP_VENDOR, _hash):
            m_props.vendor = iter.value().toString();
            break;
        case CONNECT(UPOWER_DEVICE_DBUS_PROP_MODEL, _hash):
            m_props.model = iter.value().toString();
            break;
        case CONNECT(UPOWER_DEVICE_DBUS_PROP_SERIAL, _hash):
            m_props.serial = iter.value().toString();
            break;
        case CONNECT(UPOWER_DEVICE_DBUS_PROP_UPDATE_TIME, _hash):
            m_props.updateTime = iter.value().toULongLong();
            break;
        case CONNECT(UPOWER_DEVICE_DBUS_PROP_TYPE, _hash):
            m_props.type = iter.value().toUInt();
            break;
        case CONNECT(UPOWER_DEVICE_DBUS_PROP_POWER_SUPPLY, _hash):
            m_props.powerSupply = iter.value().toBool();
            break;
        case CONNECT(UPOWER_DEVICE_DBUS_PROP_HAS_HISTORY, _hash):
            m_props.hasHistory = iter.value().toBool();
            break;
        case CONNECT(UPOWER_DEVICE_DBUS_PROP_HAS_STATISTICS, _hash):
            m_props.hasStatistics = iter.value().toBool();
            break;
        case CONNECT(UPOWER_DEVICE_DBUS_PROP_ONLINE, _hash):
            m_props.online = iter.value().toBool();
            break;
        case CONNECT(UPOWER_DEVICE_DBUS_PROP_ENERGY, _hash):
            m_props.energy = iter.value().toDouble();
            break;
        case CONNECT(UPOWER_DEVICE_DBUS_PROP_ENERGY_EMPTY, _hash):
            m_props.energyEmpty = iter.value().toDouble();
            break;
        case CONNECT(UPOWER_DEVICE_DBUS_PROP_ENERGY_FULL, _hash):
            m_props.energyFull = iter.value().toDouble();
            break;
        case CONNECT(UPOWER_DEVICE_DBUS_PROP_ENERGY_FULL_DESIGN, _hash):
            m_props.energyFullDesign = iter.value().toDouble();
            break;
        case CONNECT(UPOWER_DEVICE_DBUS_PROP_ENERGY_RATE, _hash):
            m_props.energyRate = iter.value().toDouble();
            break;
        case CONNECT(UPOWER_DEVICE_DBUS_PROP_VOLTAGE, _hash):
            m_props.voltage = iter.value().toDouble();
            break;
        case CONNECT(UPOWER_DEVICE_DBUS_PROP_LUMINOSITY, _hash):
            m_props.luminosity = iter.value().toDouble();
            break;
        case CONNECT(UPOWER_DEVICE_DBUS_PROP_TIME_TO_EMPTY, _hash):
            m_props.timeToEmpty = iter.value().toLongLong();
            break;
        case CONNECT(UPOWER_DEVICE_DBUS_PROP_TIME_TO_FULL, _hash):
            m_props.timeToFull = iter.value().toLongLong();
            break;
        case CONNECT(UPOWER_DEVICE_DBUS_PROP_PERCENTAGE, _hash):
            m_props.percentage = iter.value().toDouble();
            break;
        case CONNECT(UPOWER_DEVICE_DBUS_PROP_TEMPERATURE, _hash):
            m_props.temperature = iter.value().toDouble();
            break;
        case CONNECT(UPOWER_DEVICE_DBUS_PROP_IS_PRESENT, _hash):
            m_props.isPresent = iter.value().toBool();
            break;
        case CONNECT(UPOWER_DEVICE_DBUS_PROP_STATE, _hash):
            m_props.state = iter.value().toUInt();
            break;
        case CONNECT(UPOWER_DEVICE_DBUS_PROP_IS_RECHARGEABLE, _hash):
            m_props.isRechargeable = iter.value().toBool();
            break;
        case CONNECT(UPOWER_DEVICE_DBUS_PROP_CAPACITY, _hash):
            m_props.capacity = iter.value().toDouble();
            break;
        case CONNECT(UPOWER_DEVICE_DBUS_PROP_TECHNOLOGY, _hash):
            m_props.technology = iter.value().toUInt();
            break;
        case CONNECT(UPOWER_DEVICE_DBUS_PROP_WARNING_LEVEL, _hash):
            m_props.warningLevel = iter.value().toUInt();
            break;
        case CONNECT(UPOWER_DEVICE_DBUS_PROP_BATTERY_LEVEL, _hash):
            m_props.batteryLevel = iter.value().toUInt();
            break;
        case CONNECT(UPOWER_DEVICE_DBUS_PROP_ICON_NAME, _hash):
            m_props.iconName = iter.value().toString();
            break;
        default:
            break;
        }
    }

    Q_EMIT propsChanged(oldProps, m_props);
}
}  // namespace Kiran