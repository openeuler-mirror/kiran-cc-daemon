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

#include "power-event-control.h"
#include <QGSettings>
#include <QTimer>
#include "../backlight/power-backlight-interface.h"
#include "../backlight/power-backlight.h"
#include "../save/power-save.h"
#include "../wrapper/power-screensaver.h"
#include "../wrapper/power-upower-device.h"
#include "../wrapper/power-upower.h"
#include "../wrapper/power-wrapper-manager.h"
#include "power-event-button.h"

namespace Kiran
{
#define POWER_CRITICAL_ACTION_DELAY 20
PowerEventControl::PowerEventControl(PowerWrapperManager* wrapperManager,
                                     PowerBacklight* backlight) : m_wrapperManager(wrapperManager),
                                                                  m_backlight(backlight),
                                                                  m_lidClosedThrottle(0),
                                                                  m_displayDimmedSet(false)
{
    m_eventButton = new PowerEventButton(this);
    m_backlightKbd = backlight->getBacklightDevice(PowerDeviceType::POWER_DEVICE_TYPE_KBD);
    m_kbdLastNozeroBrightness = m_backlightKbd->getBrightness();
    m_backlightMonitor = backlight->getBacklightDevice(PowerDeviceType::POWER_DEVICE_TYPE_MONITOR);
    m_upowerClient = m_wrapperManager->getDefaultUpower();
    m_screensaver = m_wrapperManager->getDefaultScreensaver();
    m_powerSettings = new QGSettings(POWER_SCHEMA_ID, "", this);
}

PowerEventControl::~PowerEventControl()
{
}

PowerEventControl* PowerEventControl::m_instance = nullptr;
void PowerEventControl::globalInit(PowerWrapperManager* wrapperManager,
                                   PowerBacklight* backlight)
{
    m_instance = new PowerEventControl(wrapperManager, backlight);
    m_instance->init();
}

void PowerEventControl::init()
{
    m_eventButton->init();

    connect(m_eventButton, &PowerEventButton::buttonChanged, this, &PowerEventControl::processButtonChanged);
    connect(m_backlightKbd, &PowerBacklightPercentage::brightnessChanged, this, &PowerEventControl::processKbdBrightnessChanged);
    connect(m_upowerClient.get(), &PowerUPower::deviceStatusChanged, this, &PowerEventControl::processDeviceStatusChanged);
}

void PowerEventControl::chargingEvent()
{
    // 未设置过显示器变暗操作，则不要取做恢复操作，因为可能恢复的是其他场景设置的变暗操作。
    RETURN_IF_FALSE(m_displayDimmedSet);
    PowerSave::getInstance()->doDisplayRestoreDimmed();
    m_displayDimmedSet = false;

    PowerSave::getInstance()->doCpuRestoreSaver();
}

void PowerEventControl::dischargingEvent(QSharedPointer<PowerUPowerDevice> device)
{
    const UPowerDeviceProps& deviceProps = device->getProps();

    // 如果拔掉了电源线，则需要重新判断当前电量情况，然后进行节能操作。
    switch (deviceProps.warningLevel)
    {
    case UpDeviceLevel::UP_DEVICE_LEVEL_LOW:
        chargeLowEvent(device);
        break;
    default:
        break;
    }
}

void PowerEventControl::chargeLowEvent(QSharedPointer<PowerUPowerDevice> device)
{
    const UPowerDeviceProps& deviceProps = device->getProps();

    // 如果电池电量过低，但是未使用则忽略
    if (deviceProps.type == UP_DEVICE_KIND_BATTERY &&
        !m_upowerClient->getOnBattery())
    {
        return;
    }

    // 执行电量过低时显示器变暗
    auto chargeLowDimmedEnabled = m_powerSettings->get(POWER_SCHEMA_ENABLE_CHARGE_LOW_DIMMED).toBool();
    // 这里必须要判断当前是否处于变暗状态。如果当前已经处于变暗状态，调用do_display_dimmed函数会导致display_dimmed_set_置为false。
    if (chargeLowDimmedEnabled && !PowerSave::getInstance()->isDisplayDimmed())
    {
        m_displayDimmedSet = PowerSave::getInstance()->doDisplayDimmed();
    }

    // 执行电量过低时计算机进入节能模式
    auto chargeLowSaverEnabled = m_powerSettings->get(POWER_SCHEMA_ENABLE_CHARGE_LOW_SAVER).toBool();
    if (chargeLowSaverEnabled)
    {
        PowerSave::getInstance()->doCpuSaver();
    }
}

void PowerEventControl::chargeActionEvent(QSharedPointer<PowerUPowerDevice> device)
{
    const UPowerDeviceProps& device_props = device->getProps();

    // 如果电池电量过低，但是未使用则忽略
    if (device_props.type == UP_DEVICE_KIND_BATTERY &&
        !m_upowerClient->getOnBattery())
    {
        return;
    }

    PowerAction action = PowerAction::POWER_ACTION_NOTHING;

    switch (device_props.type)
    {
    case UP_DEVICE_KIND_BATTERY:
        action = PowerAction(m_powerSettings->get(POWER_SCHEMA_BATTERY_CRITICAL_ACTION).toInt());
        break;
    case UP_DEVICE_KIND_UPS:
        action = PowerAction(m_powerSettings->get(POWER_SCHEMA_UPS_CRITICAL_ACTION).toInt());
        break;
        // Ignore other type
    default:
        return;
    }

    // 电量过低执行节能操作，延时执行让用户处理一些紧急事务，notification模块中会进行提示
    QTimer::singleShot(POWER_CRITICAL_ACTION_DELAY * 1000, std::bind(&PowerEventControl::doChargeCriticalAction, this, action));
}

void PowerEventControl::doChargeCriticalAction(PowerAction action)
{
    QString error;
    if (!PowerSave::getInstance()->doSave(action, error))
    {
        KLOG_WARNING(power) << error;
    }
}

void PowerEventControl::processButtonChanged(PowerEvent evnet)
{
    QString error;
    PowerAction action = PowerAction::POWER_ACTION_NOTHING;

    switch (evnet)
    {
    case POWER_EVENT_RELEASE_POWEROFF:
    {
        action = PowerAction(m_powerSettings->get(POWER_SCHEMA_BUTTON_POWER_ACTION).toInt());
        PowerSave::getInstance()->doSave(action, error);
        break;
    }
    case POWER_EVENT_PRESSED_SLEEP:
    case POWER_EVENT_PRESSED_SUSPEND:
    {
        action = PowerAction(m_powerSettings->get(POWER_SCHEMA_BUTTON_SUSPEND_ACTION).toInt());
        PowerSave::getInstance()->doSave(action, error);
        break;
    }
    case POWER_EVENT_PRESSED_HIBERNATE:
    {
        action = PowerAction(m_powerSettings->get(POWER_SCHEMA_BUTTON_HIBERNATE_ACTION).toInt());
        PowerSave::getInstance()->doSave(action, error);
        break;
    }
    case POWER_EVENT_LID_OPEN:
    {
        PowerSave::getInstance()->doSave(PowerAction::POWER_ACTION_DISPLAY_ON, error);
        if (m_lidClosedThrottle)
        {
            m_screensaver->removeThrottle(m_lidClosedThrottle);
            m_lidClosedThrottle = 0;
        }
        break;
    }
    case POWER_EVENT_LID_CLOSED:
    {
        action = PowerAction(m_powerSettings->get(POWER_SCHEMA_LID_CLOSED_ACTION).toInt());
        PowerSave::getInstance()->doSave(action, error);
        if (m_lidClosedThrottle)
        {
            m_screensaver->removeThrottle(m_lidClosedThrottle);
        }
        m_lidClosedThrottle = m_screensaver->addThrottle("Laptop lid is closed");
        break;
    }
    case POWER_EVENT_PRESSED_BRIGHT_UP:
        m_backlightMonitor->brightnessUp();
        break;
    case POWER_EVENT_PRESSED_BRIGHT_DOWN:
        m_backlightMonitor->brightnessDown();
        break;
    case POWER_EVENT_PRESSED_KBD_BRIGHT_UP:
        m_backlightKbd->brightnessUp();
        break;
    case POWER_EVENT_PRESSED_KBD_BRIGHT_DOWN:
        m_backlightKbd->brightnessDown();
        break;
    case POWER_EVENT_PRESSED_KBD_BRIGHT_TOGGLE:
    {
        if (m_backlightKbd->getBrightness() > 0)
        {
            m_backlightKbd->setBrightness(0);
        }
        else
        {
            m_backlightKbd->setBrightness(m_kbdLastNozeroBrightness);
        }
        break;
    }
    case POWER_EVENT_PRESSED_LOCK:
        m_screensaver->lock();
        break;
    default:
        break;
    }

    if (error.length() > 0)
    {
        KLOG_WARNING(power) << error;
    }
}

void PowerEventControl::processKbdBrightnessChanged(int32_t brightness_value)
{
    if (brightness_value > 0)
    {
        m_kbdLastNozeroBrightness = brightness_value;
    }
}

void PowerEventControl::processDeviceStatusChanged(QSharedPointer<PowerUPowerDevice> device, int event)
{
    switch (event)
    {
    case UPowerDeviceEvent::UPOWER_DEVICE_EVENT_CHARGING:
        chargingEvent();
        break;
    case UPowerDeviceEvent::UPOWER_DEVICE_EVENT_DISCHARGING:
        dischargingEvent(device);
        break;
    case UPowerDeviceEvent::UPOWER_DEVICE_EVENT_CHARGE_LOW:
        chargeLowEvent(device);
        break;
    case UPowerDeviceEvent::UPOWER_DEVICE_EVENT_CHARGE_ACTION:
        chargeActionEvent(device);
        break;
    default:
        break;
    }
}

}  // namespace Kiran