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

#include "power-backlight-monitors-controller.h"
#include <QGSettings>
#include <QGuiApplication>
#include <QThread>
#include "power-backlight-monitors-tool.h"
#include "power-backlight-monitors-x11.h"

namespace Kiran
{
PowerBacklightMonitorsController::PowerBacklightMonitorsController(QObject *parent) : PowerBacklightPercentage(parent),
                                                                                      m_brightnessPercentage(-1)
{
    m_settings = new QGSettings(POWER_SCHEMA_ID, "", this);
}

void PowerBacklightMonitorsController::init()
{
    loadBacklightMonitors();
    m_brightnessPercentage = getBrightness();
}

bool PowerBacklightMonitorsController::setBrightness(int32_t percentage)
{
    auto monitors = m_backlightMonitors->getMonitors();
    for (auto &monitor : monitors)
    {
        RETURN_VAL_IF_FALSE(setBrightnessPercentage(monitor, percentage), false);
    }
    updateCachedBrightness();
    return true;
}

int32_t PowerBacklightMonitorsController::getBrightness()
{
    auto monitors = m_backlightMonitors->getMonitors();
    for (auto &monitor : monitors)
    {
        auto percentage = getBrightnessPercentage(monitor);
        RETURN_VAL_IF_TRUE(percentage >= 0, percentage);
    }
    return -1;
}

bool PowerBacklightMonitorsController::brightnessUp()
{
    auto monitors = m_backlightMonitors->getMonitors();
    for (auto &monitor : monitors)
    {
        brightnessValueUp(monitor);
    }
    return true;
}

bool PowerBacklightMonitorsController::brightnessDown()
{
    auto monitors = m_backlightMonitors->getMonitors();
    for (auto &monitor : monitors)
    {
        brightnessValueDown(monitor);
    }
    return true;
}

void PowerBacklightMonitorsController::loadBacklightMonitors()
{
    auto monitorBacklightPolicy = m_settings->get(POWER_SCHEMA_MONITOR_BACKLIGHT_POLICY).toString();

    switch (shash(monitorBacklightPolicy.toUtf8().data()))
    {
    case "tool"_hash:
        m_backlightMonitors = QSharedPointer<PowerBacklightMonitorsTool>::create();
        break;
    case "x11"_hash:
        m_backlightMonitors = QSharedPointer<PowerBacklightMonitorsX11>::create();
        break;
    default:
    {
        if (PowerBacklightMonitorsTool::supportBacklight() || qGuiApp->platformName() != "xcb")
        {
            m_backlightMonitors = QSharedPointer<PowerBacklightMonitorsTool>::create();
        }
        else
        {
            m_backlightMonitors = QSharedPointer<PowerBacklightMonitorsX11>::create();
        }
    }
    }

    m_backlightMonitors->init();
    connect(m_backlightMonitors.data(), SIGNAL(monitorChanged()), this, SLOT(updateCachedBrightness()));
    connect(m_backlightMonitors.data(), SIGNAL(brightnessChanged()), this, SLOT(updateCachedBrightness()));
}

bool PowerBacklightMonitorsController::setBrightnessPercentage(QSharedPointer<PowerBacklightAbsolute> absoluteMonitor, int32_t percentage)
{
    int32_t brightnessMin = -1;
    int32_t brightnessMax = -1;

    auto brightnessCurrentValue = absoluteMonitor->getBrightnessValue();
    RETURN_VAL_IF_TRUE(brightnessCurrentValue < 0, false);
    RETURN_VAL_IF_FALSE(absoluteMonitor->getBrightnessRange(brightnessMin, brightnessMax), false);
    RETURN_VAL_IF_TRUE(brightnessMax == brightnessMin, false);

    auto brightnessSetValue = brightnessPercent2Discrete(percentage, (brightnessMax - brightnessMin) + 1);
    KLOG_INFO(power) << "Brightness,min value:" << brightnessMin
                     << ", max value:" << brightnessMax
                     << ", current value:" << brightnessCurrentValue
                     << ", set value:" << brightnessSetValue
                     << ", set value percent:" << percentage << ".";

    brightnessSetValue = std::max(brightnessSetValue, brightnessMin);
    brightnessSetValue = std::min(brightnessSetValue, brightnessMax);

    if (brightnessCurrentValue == brightnessSetValue)
    {
        KLOG_DEBUG(power) << "The set brightness value is equal to current value.";
        return true;
    }

    // 一些背光控制器的亮度增加和减少是按照一定倍数进行的，例如macbook pro是每次增加5%的亮度值
    auto step = getBrightnessStep(std::abs(brightnessSetValue - brightnessCurrentValue));
    KLOG_INFO(power) << "Using step of" << step;

    if (brightnessCurrentValue < brightnessSetValue)
    {
        for (int32_t i = brightnessCurrentValue; i <= brightnessSetValue; i += step)
        {
            KLOG_INFO(power) << "Brightness up to" << QString("%1%").arg(i);

            if (!absoluteMonitor->setBrightnessValue(i))
            {
                break;
            }
            if (i + step <= brightnessSetValue)
            {
                QThread::usleep(5000);
            }
        }
    }
    else
    {
        for (int32_t i = brightnessCurrentValue; i >= brightnessSetValue; i -= step)
        {
            KLOG_INFO(power) << "Brightness down to" << QString("%1%").arg(i);

            if (!absoluteMonitor->setBrightnessValue(i))
            {
                break;
            }
            if (i - step >= brightnessSetValue)
            {
                QThread::usleep(5000);
            }
        }
    }
    return true;
}

int32_t PowerBacklightMonitorsController::getBrightnessPercentage(QSharedPointer<PowerBacklightAbsolute> absoluteMonitor)
{
    int32_t brightnessMin = -1;
    int32_t brightnessMax = -1;

    auto brightnessValue = absoluteMonitor->getBrightnessValue();
    RETURN_VAL_IF_TRUE(brightnessValue < 0, -1);
    RETURN_VAL_IF_FALSE(absoluteMonitor->getBrightnessRange(brightnessMin, brightnessMax), -1);
    RETURN_VAL_IF_TRUE(brightnessMin >= brightnessMax, -1);

    KLOG_DEBUG(power) << "Output brightness info: value" << brightnessValue << ", min" << brightnessMin << ", max" << brightnessMax;

    int32_t brightnessLevel = brightnessMax - brightnessMin + 1;
    auto percentage = brightnessDiscrete2Percent(brightnessValue, brightnessLevel);
    KLOG_DEBUG(power) << "Brightness discrete to percent" << percentage;
    return percentage;
}

bool PowerBacklightMonitorsController::brightnessValueUp(QSharedPointer<PowerBacklightAbsolute> absoluteMonitor)
{
    int32_t brightnessMin = -1;
    int32_t brightnessMax = -1;

    auto brightnessCurrentValue = absoluteMonitor->getBrightnessValue();
    RETURN_VAL_IF_TRUE(brightnessCurrentValue < 0, false);
    RETURN_VAL_IF_FALSE(absoluteMonitor->getBrightnessRange(brightnessMin, brightnessMax), false);
    RETURN_VAL_IF_TRUE(brightnessMax == brightnessMin, false);

    RETURN_VAL_IF_TRUE(brightnessCurrentValue == brightnessMax, true);

    brightnessCurrentValue += getBrightnessStep((brightnessMax - brightnessMin) + 1);
    brightnessCurrentValue = std::min(brightnessCurrentValue, brightnessMax);
    return absoluteMonitor->setBrightnessValue(brightnessCurrentValue);
}

bool PowerBacklightMonitorsController::brightnessValueDown(QSharedPointer<PowerBacklightAbsolute> absoluteMonitor)
{
    int32_t brightnessMin = -1;
    int32_t brightnessMax = -1;

    auto brightnessCurrentValue = absoluteMonitor->getBrightnessValue();
    RETURN_VAL_IF_TRUE(brightnessCurrentValue < 0, false);
    RETURN_VAL_IF_FALSE(absoluteMonitor->getBrightnessRange(brightnessMin, brightnessMax), false);
    RETURN_VAL_IF_TRUE(brightnessMax == brightnessMin, false);

    RETURN_VAL_IF_TRUE(brightnessCurrentValue == brightnessMin, true);

    brightnessCurrentValue -= getBrightnessStep((brightnessMax - brightnessMin) + 1);
    brightnessCurrentValue = std::max(brightnessCurrentValue, brightnessMin);
    return absoluteMonitor->setBrightnessValue(brightnessCurrentValue);
}

int32_t PowerBacklightMonitorsController::brightnessDiscrete2Percent(int32_t discrete, int32_t levels)
{
    RETURN_VAL_IF_TRUE(discrete > levels, 100);
    RETURN_VAL_IF_TRUE(levels <= 1, 0);
    return (int32_t)(((double)discrete * (100.0 / (double)(levels - 1))) + 0.5);
}

int32_t PowerBacklightMonitorsController::brightnessPercent2Discrete(int32_t percentage, int32_t levels)
{
    RETURN_VAL_IF_TRUE(percentage > 100, levels);
    RETURN_VAL_IF_TRUE(levels == 0, 0);

    return (int32_t)((((double)percentage * (double)(levels - 1)) / 100.0) + 0.5);
}

int32_t PowerBacklightMonitorsController::getBrightnessStep(uint32_t levels)
{
    if (levels > 20)
    {
        return levels / 20;
    }
    return 1;
}

void PowerBacklightMonitorsController::updateCachedBrightness()
{
    auto brightnessPercentage = getBrightness();
    if (brightnessPercentage != m_brightnessPercentage)
    {
        m_brightnessPercentage = brightnessPercentage;
        Q_EMIT brightnessChanged(m_brightnessPercentage);
    }
}

}  // namespace Kiran