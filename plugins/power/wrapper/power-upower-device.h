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

class QDBusInterface;
class QDBusMessage;

namespace Kiran
{
struct UPowerDeviceProps
{
    QString nativePath;
    QString vendor;
    QString model;
    QString serial;
    uint64_t updateTime;
    uint32_t type;
    bool powerSupply;
    bool hasHistory;
    bool hasStatistics;
    bool online;
    double energy;
    double energyEmpty;
    double energyFull;
    double energyFullDesign;
    double energyRate;
    double voltage;
    double luminosity;
    int64_t timeToEmpty;
    int64_t timeToFull;
    double percentage;
    double temperature;
    bool isPresent;
    uint32_t state;
    bool isRechargeable;
    double capacity;
    uint32_t technology;
    uint32_t warningLevel;
    uint32_t batteryLevel;
    QString iconName;
};

enum UPowerDeviceEvent
{
    // 电池未供电（充电中）
    UPOWER_DEVICE_EVENT_CHARGING,
    // 电池供电中（放电中）
    UPOWER_DEVICE_EVENT_DISCHARGING,
    // 电量充满
    UPOWER_DEVICE_EVENT_FULLY_CHARGED,
    // 电量过低（电量为10%）
    UPOWER_DEVICE_EVENT_CHARGE_LOW,
    // 电量过低（电量为3%）
    UPOWER_DEVICE_EVENT_CHARGE_CRITICAL,
    // 电量过低（电量为2%）
    UPOWER_DEVICE_EVENT_CHARGE_ACTION,

};

enum UpDeviceKind
{
    UP_DEVICE_KIND_UNKNOWN,
    UP_DEVICE_KIND_LINE_POWER,
    UP_DEVICE_KIND_BATTERY,
    UP_DEVICE_KIND_UPS,
    UP_DEVICE_KIND_MONITOR,
    UP_DEVICE_KIND_MOUSE,
    UP_DEVICE_KIND_KEYBOARD,
    UP_DEVICE_KIND_PDA,
    UP_DEVICE_KIND_PHONE,
    UP_DEVICE_KIND_MEDIA_PLAYER,
    UP_DEVICE_KIND_TABLET,
    UP_DEVICE_KIND_COMPUTER,
    UP_DEVICE_KIND_GAMING_INPUT,
    UP_DEVICE_KIND_LAST
};

enum UpDeviceState
{
    UP_DEVICE_STATE_UNKNOWN,
    // 电池未供电（充电中）
    UP_DEVICE_STATE_CHARGING,
    // 电池供电中（放电中）
    UP_DEVICE_STATE_DISCHARGING,
    UP_DEVICE_STATE_EMPTY,
    UP_DEVICE_STATE_FULLY_CHARGED,
    UP_DEVICE_STATE_PENDING_CHARGE,
    UP_DEVICE_STATE_PENDING_DISCHARGE,
    UP_DEVICE_STATE_LAST
};

enum UpDeviceLevel
{
    UP_DEVICE_LEVEL_UNKNOWN,
    UP_DEVICE_LEVEL_NONE,
    UP_DEVICE_LEVEL_DISCHARGING,
    UP_DEVICE_LEVEL_LOW,
    UP_DEVICE_LEVEL_CRITICAL,
    UP_DEVICE_LEVEL_ACTION,
    UP_DEVICE_LEVEL_NORMAL,
    UP_DEVICE_LEVEL_HIGH,
    UP_DEVICE_LEVEL_FULL,
    UP_DEVICE_LEVEL_LAST
};

class PowerUPowerDevice : public QObject
{
    Q_OBJECT

public:
    PowerUPowerDevice(const QString& objectPath);
    virtual ~PowerUPowerDevice(){};

    // 获取设备所有属性
    const UPowerDeviceProps& getProps() { return m_props; };

    // 获取设备类型字符串
    QString getKindString() { return kind2str(UpDeviceKind(m_props.type)); };

    // 获取设备的dbus object path
    const QString& getObjectPath() { return m_objectPath; };

    // 属性发生变化，参数分别为返回值、旧的属性和新的属性
Q_SIGNALS:
    void propsChanged(const UPowerDeviceProps& oldProps, const UPowerDeviceProps& newProps);

private:
    void loadDeviceProps();
    QString kind2str(UpDeviceKind typeEnum);

private Q_SLOTS:
    void processPropertiesChanged(const QDBusMessage& message);

private:
    QDBusInterface* m_upowerDeviceInterface;
    QString m_objectPath;
    UPowerDeviceProps m_props;
};

using PowerUPowerDeviceList = QList<QSharedPointer<PowerUPowerDevice>>;
}  // namespace Kiran