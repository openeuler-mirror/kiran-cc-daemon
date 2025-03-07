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

#include "power-manager.h"
#include <QDBusConnection>
#include <QGSettings>
#include <QJsonDocument>
#include <QJsonObject>
#include "backlight/power-backlight-interface.h"
#include "backlight/power-backlight.h"
#include "power-i.h"
#include "power-utils.h"
#include "poweradaptor.h"
#include "wrapper/power-profiles.h"
#include "wrapper/power-upower.h"
#include "wrapper/power-wrapper-manager.h"

namespace Kiran
{
PowerManager::PowerManager(PowerWrapperManager* wrapperManager,
                           PowerBacklight* backlight) : m_wrapperManager(wrapperManager),
                                                        m_backlight(backlight)
{
    m_powerAdaptor = new PowerAdaptor(this);
    m_powerSettings = new QGSettings(POWER_SCHEMA_ID, "", this);
    m_upowerClient = m_wrapperManager->getDefaultUpower();
    m_profiles = m_wrapperManager->getDefaultProfiles();

    qDBusRegisterMetaType<IdleActionInfo>();
}

PowerManager::~PowerManager()
{
}

#define SEND_PROPERTY_NOTIFY(property, propertyHump)                          \
    QVariantMap changedProperties;                                            \
    changedProperties.insert(QStringLiteral(#property), get##propertyHump()); \
                                                                              \
    QDBusMessage signalMessage = QDBusMessage::createSignal(                  \
        POWER_OBJECT_PATH,                                                    \
        QStringLiteral("org.freedesktop.DBus.Properties"),                    \
        QStringLiteral("PropertiesChanged"));                                 \
                                                                              \
    signalMessage.setArguments({                                              \
        POWER_DBUS_INTERFACE_NAME,                                            \
        changedProperties,                                                    \
        QStringList(),                                                        \
    });                                                                       \
    QDBusConnection::sessionBus().send(signalMessage);

int PowerManager::getActiveProfile() const
{
    return m_profiles->getActiveProfile();
}

bool PowerManager::getChargeLowDimmedEnabled() const
{
    return m_powerSettings->get(POWER_SCHEMA_ENABLE_CHARGE_LOW_DIMMED).toBool();
}

bool PowerManager::getChargeLowSaverEnabled() const
{
    return m_powerSettings->get(POWER_SCHEMA_ENABLE_CHARGE_LOW_SAVER).toBool();
}

bool PowerManager::getDisplayIdleDimmedEnabled() const
{
    return m_powerSettings->get(POWER_SCHEMA_ENABLE_DISPLAY_IDLE_DIMMED).toBool();
}

bool PowerManager::getLidIsPresent() const
{
    return m_upowerClient->getLidIsPresent();
}

bool PowerManager::getOnBattery() const
{
    return m_upowerClient->getOnBattery();
}

void PowerManager::setActiveProfile(int activeProfile)
{
    m_profiles->switchProfile(activeProfile);
    SEND_PROPERTY_NOTIFY(ActiveProfile, ActiveProfile);
}

void PowerManager::setChargeLowDimmedEnabled(bool enabled)
{
    m_powerSettings->set(POWER_SCHEMA_ENABLE_CHARGE_LOW_DIMMED, enabled);
    SEND_PROPERTY_NOTIFY(ChargeLowDimmedEnabled, ChargeLowDimmedEnabled);
}
void PowerManager::setChargeLowSaverEnabled(bool enabled)
{
    m_powerSettings->set(POWER_SCHEMA_ENABLE_CHARGE_LOW_SAVER, enabled);
    SEND_PROPERTY_NOTIFY(ChargeLowSaverEnabled, ChargeLowSaverEnabled);
}
void PowerManager::setDisplayIdleDimmedEnabled(bool enabled)
{
    m_powerSettings->set(POWER_SCHEMA_ENABLE_DISPLAY_IDLE_DIMMED, enabled);
    SEND_PROPERTY_NOTIFY(DisplayIdleDimmedEnabled, DisplayIdleDimmedEnabled);
}

void PowerManager::setLidIsPresent(bool lidIsPresent)
{
    SEND_PROPERTY_NOTIFY(LidIsPresent, LidIsPresent);
}

void PowerManager::setOnBattery(bool onBattery)
{
    // 这里只做信号通知
    SEND_PROPERTY_NOTIFY(OnBattery, OnBattery);
}

void PowerManager::EnableChargeLowDimmed(bool enabled)
{
    setChargeLowDimmedEnabled(enabled);
}

void PowerManager::EnableChargeLowSaver(bool enabled)
{
    setChargeLowSaverEnabled(enabled);
}

void PowerManager::EnableDisplayIdleDimmed(bool enabled)
{
    setDisplayIdleDimmedEnabled(enabled);
}

int PowerManager::GetBrightness(int device)
{
    int32_t brightnessPercentage = -1;

    switch (device)
    {
    case PowerDeviceType::POWER_DEVICE_TYPE_MONITOR:
    {
        auto monitor = PowerBacklight::getInstance()->getBacklightDevice(PowerDeviceType::POWER_DEVICE_TYPE_MONITOR);
        brightnessPercentage = monitor->getBrightness();
        break;
    }
    case PowerDeviceType::POWER_DEVICE_TYPE_KBD:
    {
        auto kbd = PowerBacklight::getInstance()->getBacklightDevice(PowerDeviceType::POWER_DEVICE_TYPE_KBD);
        brightnessPercentage = kbd->getBrightness();
        break;
    }
    default:
        DBUS_ERROR_REPLY_AND_RETVAL(-1, CCErrorCode::ERROR_POWER_DEVICE_UNSUPPORTED_4);
    }

    return brightnessPercentage;
}

int PowerManager::GetEventAction(int event)
{
    QString actionStr;

    switch (event)
    {
    case PowerEvent::POWER_EVENT_RELEASE_POWEROFF:
        actionStr = m_powerSettings->get(POWER_SCHEMA_BUTTON_POWER_ACTION).toString();
        break;
    case PowerEvent::POWER_EVENT_PRESSED_SLEEP:
    case PowerEvent::POWER_EVENT_PRESSED_SUSPEND:
        actionStr = m_powerSettings->get(POWER_SCHEMA_BUTTON_SUSPEND_ACTION).toString();
        break;
    case PowerEvent::POWER_EVENT_PRESSED_HIBERNATE:
        actionStr = m_powerSettings->get(POWER_SCHEMA_BUTTON_HIBERNATE_ACTION).toString();
        break;
    case PowerEvent::POWER_EVENT_LID_CLOSED:
        actionStr = m_powerSettings->get(POWER_SCHEMA_LID_CLOSED_ACTION).toString();
        break;
    case PowerEvent::POWER_EVENT_BATTERY_CHARGE_ACTION:
        actionStr = m_powerSettings->get(POWER_SCHEMA_BATTERY_CRITICAL_ACTION).toString();
        break;
    default:
        DBUS_ERROR_REPLY_AND_RETVAL(-1, CCErrorCode::ERROR_POWER_EVENT_UNSUPPORTED_2);
    }
    return PowerUtils::eventActionStr2Enum(actionStr);
}

IdleActionInfo PowerManager::GetIdleAction(int device, int supply)
{
    int idleTimeout = 0;
    int action = PowerAction::POWER_ACTION_NOTHING;
    QString actionStr;

    switch (device)
    {
    case PowerDeviceType::POWER_DEVICE_TYPE_COMPUTER:
    {
        switch (supply)
        {
        case PowerSupplyMode::POWER_SUPPLY_MODE_BATTERY:
            idleTimeout = m_powerSettings->get(POWER_SCHEMA_COMPUTER_BATTERY_IDLE_TIME).toInt();
            actionStr = m_powerSettings->get(POWER_SCHEMA_COMPUTER_BATTERY_IDLE_ACTION).toString();
            break;
        case PowerSupplyMode::POWER_SUPPLY_MODE_AC:
            idleTimeout = m_powerSettings->get(POWER_SCHEMA_COMPUTER_AC_IDLE_TIME).toInt();
            actionStr = m_powerSettings->get(POWER_SCHEMA_COMPUTER_AC_IDLE_ACTION).toString();
            break;
        default:
            DBUS_ERROR_REPLY_AND_RETVAL(IdleActionInfo(), CCErrorCode::ERROR_POWER_SUPPLY_MODE_UNSUPPORTED_3);
        }
        action = PowerUtils::computerActionStr2Enum(actionStr);
        break;
    }
    case PowerDeviceType::POWER_DEVICE_TYPE_BACKLIGHT:
    {
        switch (supply)
        {
        case PowerSupplyMode::POWER_SUPPLY_MODE_BATTERY:
            idleTimeout = m_powerSettings->get(POWER_SCHEMA_BACKLIGHT_BATTERY_IDLE_TIME).toInt();
            actionStr = m_powerSettings->get(POWER_SCHEMA_BACKLIGHT_BATTERY_IDLE_ACTION).toString();
            break;
        case PowerSupplyMode::POWER_SUPPLY_MODE_AC:
            idleTimeout = m_powerSettings->get(POWER_SCHEMA_BACKLIGHT_AC_IDLE_TIME).toInt();
            actionStr = m_powerSettings->get(POWER_SCHEMA_BACKLIGHT_AC_IDLE_ACTION).toString();
            break;
        default:
            DBUS_ERROR_REPLY_AND_RETVAL(IdleActionInfo(), CCErrorCode::ERROR_POWER_SUPPLY_MODE_UNSUPPORTED_4);
        }
        action = PowerUtils::monitorActionStr2Enum(actionStr);
        break;
    }
    default:
        DBUS_ERROR_REPLY_AND_RETVAL(IdleActionInfo(), CCErrorCode::ERROR_POWER_DEVICE_UNSUPPORTED_2);
        break;
    }

    return IdleActionInfo(idleTimeout, action);
}

void PowerManager::SetBrightness(int device, int brightnessPercentage)
{
    KLOG_INFO(power) << "Set brightness percentage of device"
                     << PowerUtils::deviceEnum2str(device)
                     << "to"
                     << brightnessPercentage;

    bool result = false;

    switch (device)
    {
    case PowerDeviceType::POWER_DEVICE_TYPE_MONITOR:
    {
        auto monitor = PowerBacklight::getInstance()->getBacklightDevice(PowerDeviceType::POWER_DEVICE_TYPE_MONITOR);
        result = monitor->setBrightness(brightnessPercentage);
        break;
    }
    case PowerDeviceType::POWER_DEVICE_TYPE_KBD:
    {
        auto kbd = PowerBacklight::getInstance()->getBacklightDevice(PowerDeviceType::POWER_DEVICE_TYPE_KBD);
        result = kbd->setBrightness(brightnessPercentage);
        break;
    }
    default:
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_POWER_DEVICE_UNSUPPORTED_3);
    }

    if (!result)
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_POWER_SET_BRIGHTNESS_FAILED);
    }
}

void PowerManager::SetEventAction(int event, int action)
{
    if (action < 0 || action >= PowerAction::POWER_ACTION_LAST)
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_POWER_UNKNOWN_ACTION_2);
    }

    auto actionStr = PowerUtils::eventActionEnum2Str(action);

    switch (event)
    {
    case PowerEvent::POWER_EVENT_RELEASE_POWEROFF:
        m_powerSettings->set(POWER_SCHEMA_BUTTON_POWER_ACTION, actionStr);
        break;
    case PowerEvent::POWER_EVENT_PRESSED_SLEEP:
    case PowerEvent::POWER_EVENT_PRESSED_SUSPEND:
        m_powerSettings->set(POWER_SCHEMA_BUTTON_SUSPEND_ACTION, actionStr);
        break;
    case PowerEvent::POWER_EVENT_PRESSED_HIBERNATE:
        m_powerSettings->set(POWER_SCHEMA_BUTTON_HIBERNATE_ACTION, actionStr);
        break;
    case PowerEvent::POWER_EVENT_LID_CLOSED:
        m_powerSettings->set(POWER_SCHEMA_LID_CLOSED_ACTION, actionStr);
        break;
    case PowerEvent::POWER_EVENT_BATTERY_CHARGE_ACTION:
        m_powerSettings->set(POWER_SCHEMA_BATTERY_CRITICAL_ACTION, actionStr);
        break;
    default:
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_POWER_EVENT_UNSUPPORTED_1);
    }
}

void PowerManager::SetIdleAction(int device, int supply, int idleTimeout, int action)
{
    KLOG_INFO(power) << "Set idle action for device" << PowerUtils::deviceEnum2str(device)
                     << "which supply=" << PowerUtils::supplyEnum2str(supply)
                     << ", idle timeout=" << idleTimeout
                     << ", action=" << PowerUtils::actionEnum2str(action);

    if (action < 0 || action >= PowerAction::POWER_ACTION_LAST)
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_POWER_UNKNOWN_ACTION_1);
    }

    switch (device)
    {
    case PowerDeviceType::POWER_DEVICE_TYPE_COMPUTER:
    {
        auto actionStr = PowerUtils::computerActionEnum2Str(action);
        switch (supply)
        {
        case PowerSupplyMode::POWER_SUPPLY_MODE_BATTERY:
            m_powerSettings->set(POWER_SCHEMA_COMPUTER_BATTERY_IDLE_TIME, idleTimeout);
            m_powerSettings->set(POWER_SCHEMA_COMPUTER_BATTERY_IDLE_ACTION, actionStr);
            break;
        case PowerSupplyMode::POWER_SUPPLY_MODE_AC:
            m_powerSettings->set(POWER_SCHEMA_COMPUTER_AC_IDLE_TIME, idleTimeout);
            m_powerSettings->set(POWER_SCHEMA_COMPUTER_AC_IDLE_ACTION, actionStr);
            break;
        default:
            DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_POWER_SUPPLY_MODE_UNSUPPORTED_1);
        }
        break;
    }
    case PowerDeviceType::POWER_DEVICE_TYPE_BACKLIGHT:
    {
        auto actionStr = PowerUtils::monitorActionEnum2Str(action);
        switch (supply)
        {
        case PowerSupplyMode::POWER_SUPPLY_MODE_BATTERY:
            m_powerSettings->set(POWER_SCHEMA_BACKLIGHT_BATTERY_IDLE_TIME, idleTimeout);
            m_powerSettings->set(POWER_SCHEMA_BACKLIGHT_BATTERY_IDLE_ACTION, actionStr);
            break;
        case PowerSupplyMode::POWER_SUPPLY_MODE_AC:
            m_powerSettings->set(POWER_SCHEMA_BACKLIGHT_AC_IDLE_TIME, idleTimeout);
            m_powerSettings->set(POWER_SCHEMA_BACKLIGHT_AC_IDLE_ACTION, actionStr);
            break;
        default:
            DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_POWER_SUPPLY_MODE_UNSUPPORTED_2);
        }
        break;
    }
    default:
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_POWER_DEVICE_UNSUPPORTED_1);
        break;
    }
}

void PowerManager::SwitchProfile(int mode)
{
    setActiveProfile(mode);
}

PowerManager* PowerManager::m_instance = nullptr;
void PowerManager::globalInit(PowerWrapperManager* wrapper_manager, PowerBacklight* backlight)
{
    m_instance = new PowerManager(wrapper_manager, backlight);
    m_instance->init();
}

void PowerManager::init()
{
    connect(m_upowerClient.get(), &PowerUPower::onBatteryChanged, this, &PowerManager::setOnBattery);
    connect(m_upowerClient.get(), &PowerUPower::lidIsPresentChanged, this, &PowerManager::setLidIsPresent);
    connect(m_powerSettings, &QGSettings::changed, this, &PowerManager::processSettingsChanged);
    connect(m_backlight, &PowerBacklight::brightnessChanged, this, &PowerManager::processBrightnessChanged);
    connect(m_profiles.get(), &PowerProfiles::activeProfileChanged, this, &PowerManager::processActiveProfileChanged);

    auto sessionConnection = QDBusConnection::sessionBus();
    if (!sessionConnection.registerService(POWER_DBUS_NAME))
    {
        KLOG_WARNING(appearance) << "Failed to register dbus name: " << POWER_DBUS_NAME;
        return;
    }

    if (!sessionConnection.registerObject(POWER_OBJECT_PATH, POWER_DBUS_INTERFACE_NAME, this))
    {
        KLOG_ERROR(appearance) << "Can't register object:" << sessionConnection.lastError();
        return;
    }
}

void PowerManager::processSettingsChanged(const QString& key)
{
    switch (shash(key.toLatin1().data()))
    {
    case CONNECT(POWER_SCHEMA_COMPUTER_BATTERY_IDLE_TIME, _hash):
    case CONNECT(POWER_SCHEMA_COMPUTER_BATTERY_IDLE_ACTION, _hash):
        Q_EMIT IdleActionChanged(PowerDeviceType::POWER_DEVICE_TYPE_COMPUTER, PowerSupplyMode::POWER_SUPPLY_MODE_BATTERY);
        break;
    case CONNECT(POWER_SCHEMA_COMPUTER_AC_IDLE_TIME, _hash):
    case CONNECT(POWER_SCHEMA_COMPUTER_AC_IDLE_ACTION, _hash):
        Q_EMIT IdleActionChanged(PowerDeviceType::POWER_DEVICE_TYPE_COMPUTER, PowerSupplyMode::POWER_SUPPLY_MODE_AC);
        break;
    case CONNECT(POWER_SCHEMA_BACKLIGHT_BATTERY_IDLE_TIME, _hash):
    case CONNECT(POWER_SCHEMA_BACKLIGHT_BATTERY_IDLE_ACTION, _hash):
        Q_EMIT IdleActionChanged(PowerDeviceType::POWER_DEVICE_TYPE_BACKLIGHT, PowerSupplyMode::POWER_SUPPLY_MODE_BATTERY);
        break;
    case CONNECT(POWER_SCHEMA_BACKLIGHT_AC_IDLE_TIME, _hash):
    case CONNECT(POWER_SCHEMA_BACKLIGHT_AC_IDLE_ACTION, _hash):
        Q_EMIT IdleActionChanged(PowerDeviceType::POWER_DEVICE_TYPE_BACKLIGHT, PowerSupplyMode::POWER_SUPPLY_MODE_AC);
        break;
    case CONNECT(POWER_SCHEMA_BUTTON_SUSPEND_ACTION, _hash):
        Q_EMIT EventActionChanged(PowerEvent::POWER_EVENT_PRESSED_SUSPEND);
        break;
    case CONNECT(POWER_SCHEMA_BUTTON_HIBERNATE_ACTION, _hash):
        Q_EMIT EventActionChanged(PowerEvent::POWER_EVENT_PRESSED_HIBERNATE);
        break;
    case CONNECT(POWER_SCHEMA_BUTTON_POWER_ACTION, _hash):
        Q_EMIT EventActionChanged(PowerEvent::POWER_EVENT_RELEASE_POWEROFF);
        break;
    case CONNECT(POWER_SCHEMA_LID_CLOSED_ACTION, _hash):
        Q_EMIT EventActionChanged(PowerEvent::POWER_EVENT_LID_CLOSED);
        break;
    default:
        break;
    }
}

void PowerManager::processBrightnessChanged(PowerBacklightPercentage* backlightDevice,
                                            int32_t brightnessValue)
{
    Q_EMIT BrightnessChanged(backlightDevice->getType());
}

void PowerManager::processActiveProfileChanged(int32_t profileMode)
{
    Q_EMIT ActiveProfileChanged(profileMode);
}

}  // namespace Kiran