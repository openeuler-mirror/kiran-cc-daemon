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

#include "power-save.h"
#include <QGSettings>
#include "../backlight/power-backlight-interface.h"
#include "../backlight/power-backlight.h"
#include "../power-utils.h"
#include "../wrapper/power-profiles.h"
#include "../wrapper/power-wrapper-manager.h"
#include "power-i.h"
#include "power-save-computer.h"
#include "power-save-dpms.h"

namespace Kiran
{
// 亮度变暗需要一个过程，这里预估为10秒以内
#define DISPLAY_DIMMED_INTERVAL 10
#define CPU_SAVER_INTERVAL 3

PowerSave::PowerSave(PowerWrapperManager* wrapperManager,
                     PowerBacklight* backlight) : m_wrapperManager(wrapperManager),
                                                  m_backlight(backlight),
                                                  m_kbdRestoreBrightness(-1),
                                                  m_monitorRestoreBrightness(-1),
                                                  m_displayDimmedTimestamp(0),
                                                  m_cpuSaverCookie(0),
                                                  m_cpuSaverTimestamp(0)
{
#ifdef WITH_DPMS_KDE
    m_dpms = new PowerSaveDpmsKDE(this);
#else
    m_dpms = new PowerSaveDpmsX11(this);
#endif
    m_powerSettings = new QGSettings(POWER_SCHEMA_ID, "", this);
    m_profiles = m_wrapperManager->getDefaultProfiles();
    m_saveComputer = new PowerSaveComputer(this);
}

PowerSave::~PowerSave()
{
}

PowerSave* PowerSave::m_instance = nullptr;
void PowerSave::globalInit(PowerWrapperManager* wrapperManager, PowerBacklight* backlight)
{
    m_instance = new PowerSave(wrapperManager, backlight);
    m_instance->init();
}

bool PowerSave::doSave(PowerAction action, QString& error)
{
    KLOG_INFO(power) << "Do power save action" << PowerUtils::actionEnum2str(action);

    switch (action)
    {
    case PowerAction::POWER_ACTION_DISPLAY_ON:
        m_dpms->setLevel(PowerDpmsLevel::POWER_DPMS_LEVEL_ON);
        break;
    case PowerAction::POWER_ACTION_DISPLAY_STANDBY:
        m_dpms->setLevel(PowerDpmsLevel::POWER_DPMS_LEVEL_STANDBY);
        break;
    case PowerAction::POWER_ACTION_DISPLAY_SUSPEND:
        m_dpms->setLevel(PowerDpmsLevel::POWER_DPMS_LEVEL_SUSPEND);
        break;
    case PowerAction::POWER_ACTION_DISPLAY_OFF:
        m_dpms->setLevel(PowerDpmsLevel::POWER_DPMS_LEVEL_OFF);
        break;
    case PowerAction::POWER_ACTION_COMPUTER_SUSPEND:
        m_saveComputer->suspend();
        break;
    case PowerAction::POWER_ACTION_COMPUTER_SHUTDOWN:
        m_saveComputer->shutdown();
        break;
    case PowerAction::POWER_ACTION_COMPUTER_HIBERNATE:
        m_saveComputer->hibernate();
        break;
    case PowerAction::POWER_ACTION_NOTHING:
        break;
    default:
        error = "Unsupported action";
        return false;
    }
    return true;
}

bool PowerSave::isDisplayDimmed()
{
    if (m_kbdRestoreBrightness != -1 || m_monitorRestoreBrightness != -1)
    {
        return true;
    }
    return false;
}

bool PowerSave::doDisplayDimmed()
{
    // 如果还处于变暗状态，则不允许重新设置，避免多个场景(计算机空闲、电量过低）下设置变暗->恢复变暗冲突
    if (isDisplayDimmed())
    {
        KLOG_INFO(power) << "The display already is dimmed status.";
        return false;
    }

    auto backlightKbd = m_backlight->getBacklightDevice(PowerDeviceType::POWER_DEVICE_TYPE_KBD);
    auto backlightMonitor = m_backlight->getBacklightDevice(PowerDeviceType::POWER_DEVICE_TYPE_MONITOR);
    auto brightnessPercentage = m_powerSettings->get(POWER_SCHEMA_DISPLAY_DIMMED_BRIGHTNESS).toInt();
    if (brightnessPercentage > 0 && brightnessPercentage <= 100)
    {
        m_displayDimmedTimestamp = time(NULL);

        auto kbd_brightness_percentage = backlightKbd->getBrightness();
        if (kbd_brightness_percentage >= 0)
        {
            backlightKbd->setBrightness(brightnessPercentage);
            m_kbdRestoreBrightness = kbd_brightness_percentage;
        }

        auto monitor_brightness_percentage = backlightMonitor->getBrightness();
        if (monitor_brightness_percentage >= 0)
        {
            backlightMonitor->setBrightness(brightnessPercentage);
            m_monitorRestoreBrightness = monitor_brightness_percentage;
        }

        KLOG_INFO(power) << "The display is dimmed.";
    }
    else
    {
        KLOG_WARNING(power) << "The brightness value is invalid, brightness:" << brightnessPercentage;
    }

    return true;
}

void PowerSave::doDisplayRestoreDimmed()
{
    RETURN_IF_TRUE(!isDisplayDimmed());

    auto backlightKbd = m_backlight->getBacklightDevice(PowerDeviceType::POWER_DEVICE_TYPE_KBD);
    auto backlightMonitor = m_backlight->getBacklightDevice(PowerDeviceType::POWER_DEVICE_TYPE_MONITOR);
    auto kbd_brightness_percentage = backlightKbd->getBrightness();

    if (kbd_brightness_percentage >= 0 && m_kbdRestoreBrightness >= 0)
    {
        backlightKbd->setBrightness(m_kbdRestoreBrightness);
        m_kbdRestoreBrightness = -1;
    }

    auto monitor_brightness_percentage = backlightMonitor->getBrightness();
    if (monitor_brightness_percentage >= 0 && m_monitorRestoreBrightness >= 0)
    {
        backlightMonitor->setBrightness(m_monitorRestoreBrightness);
        m_monitorRestoreBrightness = -1;
    }

    m_displayDimmedTimestamp = 0;

    KLOG_INFO(power) << "The display is restore dimmed.";
}

void PowerSave::doCpuSaver()
{
    if (m_cpuSaverCookie > 0)
    {
        KLOG_INFO(power) << "The cpu already is on saver mode.";
        return;
    }

    m_cpuSaverCookie = m_profiles->holdProfile(PowerProfileMode::POWER_PROFILE_MODE_SAVER, "battery or ups power low.");
    m_cpuSaverTimestamp = time(NULL);
}

void PowerSave::doCpuRestoreSaver()
{
    if (m_cpuSaverCookie > 0)
    {
        m_profiles->releaseProfile(m_cpuSaverCookie);
        m_cpuSaverCookie = 0;
    }
    m_cpuSaverTimestamp = 0;
}

void PowerSave::init()
{
    m_saveComputer->init();

    auto backlightKbd = m_backlight->getBacklightDevice(PowerDeviceType::POWER_DEVICE_TYPE_KBD);
    auto backlightMonitor = m_backlight->getBacklightDevice(PowerDeviceType::POWER_DEVICE_TYPE_MONITOR);

    connect(backlightKbd, &PowerBacklightPercentage::brightnessChanged, this, &PowerSave::processKbdBrightnessChanged);
    connect(backlightMonitor, &PowerBacklightPercentage::brightnessChanged, this, &PowerSave::processMonitorBrightnessChanged);
    connect(m_profiles.get(), &PowerProfiles::activeProfileChanged, this, &PowerSave::processActiveProfileChanged);
}

void PowerSave::processKbdBrightnessChanged(int32_t brightnessPercentage)
{
    // 亮度变暗操作结束DISPLAY_DIMMED_INTERVAL秒后，认为后续的亮度变化为手动设置，如果亮度进行了手动设置，则不再做亮度变暗恢复操作。
    if (m_displayDimmedTimestamp > 0 &&
        m_displayDimmedTimestamp + DISPLAY_DIMMED_INTERVAL < time(NULL))
    {
        KLOG_DEBUG(power) << "The keyboard brightness is changed, so ignore keyboard brightness restores.";
        m_kbdRestoreBrightness = -1;
    }
}

void PowerSave::processMonitorBrightnessChanged(int32_t brightnessPercentage)
{
    if (m_displayDimmedTimestamp > 0 &&
        m_displayDimmedTimestamp + DISPLAY_DIMMED_INTERVAL < time(NULL))
    {
        KLOG_DEBUG(power) << "The monitor brightness is changed, so ignore monitor brightness restores.";
        m_monitorRestoreBrightness = -1;
    }
}

void PowerSave::processActiveProfileChanged(int32_t profileMode)
{
    if (m_cpuSaverTimestamp > 0 &&
        m_cpuSaverTimestamp + CPU_SAVER_INTERVAL < time(NULL))
    {
        KLOG_DEBUG(power) << "The power active profile is changed, so release previous profile.";
        doCpuRestoreSaver();
    }
}
}  // namespace Kiran