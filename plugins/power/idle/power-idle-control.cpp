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

#include "power-idle-control.h"
#include <QGSettings>
#include "../backlight/power-backlight.h"
#include "../save/power-save.h"
#include "../wrapper/power-upower.h"
#include "../wrapper/power-wrapper-manager.h"
#include "lib/base/base.h"
#include "power-idle-timer.h"

namespace Kiran
{
PowerIdleControl::PowerIdleControl(PowerWrapperManager* wrapperManager,
                                   PowerBacklight* backlight) : m_wrapperManager(wrapperManager),
                                                                m_backlight(backlight),
                                                                m_computerIdleTime(0),
                                                                m_displayIdleTime(0),
                                                                m_displayDimmedSet(false)
{
    m_upowerClient = m_wrapperManager->getDefaultUpower();
    m_idleTimer = new PowerIdleTimer(this);
    m_powerSettings = new QGSettings(POWER_SCHEMA_ID, "", this);
}

PowerIdleControl::~PowerIdleControl()
{
}

PowerIdleControl* PowerIdleControl::m_instance = nullptr;
void PowerIdleControl::globalInit(PowerWrapperManager* wrapperManager, PowerBacklight* backlight)
{
    m_instance = new PowerIdleControl(wrapperManager, backlight);
    m_instance->init();
}

void PowerIdleControl::init()
{
    m_idleTimer->init();
    updateIdleTimer();

    connect(m_upowerClient.get(), &PowerUPower::onBatteryChanged, this, &PowerIdleControl::processBatteryChanged);
    connect(m_powerSettings, &QGSettings::changed, this, &PowerIdleControl::processSettingsChanged);
    connect(m_idleTimer, &PowerIdleTimer::modeChanged, this, &PowerIdleControl::processIdleModeChanged);
}

void PowerIdleControl::updateIdleTimer()
{
    if (m_upowerClient->getOnBattery())
    {
        m_computerIdleTime = m_powerSettings->get(POWER_SCHEMA_COMPUTER_BATTERY_IDLE_TIME).toInt();
        m_computerIdleAction = PowerAction(m_powerSettings->get(POWER_SCHEMA_COMPUTER_BATTERY_IDLE_ACTION).toInt());

        m_displayIdleTime = m_powerSettings->get(POWER_SCHEMA_BACKLIGHT_BATTERY_IDLE_TIME).toInt();
        m_displayIdleAction = PowerAction(m_powerSettings->get(POWER_SCHEMA_BACKLIGHT_BATTERY_IDLE_ACTION).toInt());
    }
    else
    {
        m_computerIdleTime = m_powerSettings->get(POWER_SCHEMA_COMPUTER_AC_IDLE_TIME).toInt();
        m_computerIdleAction = PowerAction(m_powerSettings->get(POWER_SCHEMA_COMPUTER_AC_IDLE_ACTION).toInt());

        m_displayIdleTime = m_powerSettings->get(POWER_SCHEMA_BACKLIGHT_AC_IDLE_TIME).toInt();
        m_displayIdleAction = PowerAction(m_powerSettings->get(POWER_SCHEMA_BACKLIGHT_AC_IDLE_ACTION).toInt());
    }

    m_idleTimer->setIdleTimeout(PowerIdleMode::POWER_IDLE_MODE_BLANK, m_displayIdleTime);
    m_idleTimer->setIdleTimeout(PowerIdleMode::POWER_IDLE_MODE_SLEEP, m_computerIdleTime);
}

void PowerIdleControl::switchToNormal()
{
    QString error;

    // 正常状态下退出显示器的节能模式
    if (!PowerSave::getInstance()->doSave(PowerAction::POWER_ACTION_DISPLAY_ON, error))
    {
        KLOG_WARNING(power) << error;
    }

    // 之前如果设置过变暗操作，则进行恢复
    if (m_displayDimmedSet)
    {
        PowerSave::getInstance()->doDisplayRestoreDimmed();
        m_displayDimmedSet = false;
    }
}

void PowerIdleControl::switchToDim()
{
    auto display_idle_dimmed_enabled = m_powerSettings->get(POWER_SCHEMA_ENABLE_DISPLAY_IDLE_DIMMED).toBool();
    // 这里必须要判断当前是否处于变暗状态。如果当前已经处于变暗状态，调用do_display_dimmed函数会导致display_dimmed_set_置为false。
    if (display_idle_dimmed_enabled && !PowerSave::getInstance()->isDisplayDimmed())
    {
        m_displayDimmedSet = PowerSave::getInstance()->doDisplayDimmed();
    }
}

void PowerIdleControl::switchToBlank()
{
    QString error;

    if (!PowerSave::getInstance()->doSave(m_displayIdleAction, error))
    {
        KLOG_WARNING(power) << error;
    }
}

void PowerIdleControl::switchToSleep()
{
    QString error;

    if (!PowerSave::getInstance()->doSave(m_computerIdleAction, error))
    {
        KLOG_WARNING(power) << error;
    }
}

void PowerIdleControl::processBatteryChanged(bool)
{
    // 电池/电源切换时，空闲超时参数需要重新设置
    updateIdleTimer();
}

void PowerIdleControl::processSettingsChanged(const QString& key)
{
    switch (shash(key.toLatin1().data()))
    {
    case CONNECT(POWER_SCHEMA_COMPUTER_BATTERY_IDLE_TIME, _hash):
    case CONNECT(POWER_SCHEMA_COMPUTER_BATTERY_IDLE_ACTION, _hash):
    case CONNECT(POWER_SCHEMA_BACKLIGHT_BATTERY_IDLE_TIME, _hash):
    case CONNECT(POWER_SCHEMA_BACKLIGHT_BATTERY_IDLE_ACTION, _hash):
    case CONNECT(POWER_SCHEMA_COMPUTER_AC_IDLE_TIME, _hash):
    case CONNECT(POWER_SCHEMA_COMPUTER_AC_IDLE_ACTION, _hash):
    case CONNECT(POWER_SCHEMA_BACKLIGHT_AC_IDLE_TIME, _hash):
    case CONNECT(POWER_SCHEMA_BACKLIGHT_AC_IDLE_ACTION, _hash):
        updateIdleTimer();
        break;
    }
}

void PowerIdleControl::processIdleModeChanged(int mode)
{
    switch (mode)
    {
    case PowerIdleMode::POWER_IDLE_MODE_NORMAL:
        switchToNormal();
        break;
    case PowerIdleMode::POWER_IDLE_MODE_DIM:
        switchToDim();
        break;
    case PowerIdleMode::POWER_IDLE_MODE_BLANK:
        switchToBlank();
        break;
    case PowerIdleMode::POWER_IDLE_MODE_SLEEP:
        switchToSleep();
        break;
    default:
        break;
    }
}

}  // namespace Kiran