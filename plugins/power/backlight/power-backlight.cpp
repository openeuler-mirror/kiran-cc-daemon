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

#include "power-backlight.h"
#include "power-backlight-interface.h"
#include "power-backlight-kbd.h"
#include "power-backlight-monitors-controller.h"

namespace Kiran
{
PowerBacklight::PowerBacklight()
{
    m_backlightMonitor = new PowerBacklightMonitorsController(this);
    m_backlightKbd = new PowerBacklightKbd(this);
}

PowerBacklight::~PowerBacklight()
{
}

PowerBacklight* PowerBacklight::m_instance = nullptr;
void PowerBacklight::globalInit()
{
    m_instance = new PowerBacklight();
    m_instance->init();
}

PowerBacklightPercentage* PowerBacklight::getBacklightDevice(PowerDeviceType device)
{
    switch (device)
    {
    case PowerDeviceType::POWER_DEVICE_TYPE_MONITOR:
        return m_backlightMonitor;
    case PowerDeviceType::POWER_DEVICE_TYPE_KBD:
        return m_backlightKbd;
    default:
        break;
    }
    return nullptr;
}

void PowerBacklight::init()
{
    m_backlightMonitor->init();
    m_backlightKbd->init();

    connect(m_backlightMonitor,
            &PowerBacklightPercentage::brightnessChanged,
            std::bind(&PowerBacklight::processBacklightBrightnessChanged, this, std::placeholders::_1, m_backlightMonitor));

    connect(m_backlightKbd,
            &PowerBacklightPercentage::brightnessChanged,
            std::bind(&PowerBacklight::processBacklightBrightnessChanged, this, std::placeholders::_1, m_backlightKbd));
}

void PowerBacklight::processBacklightBrightnessChanged(int32_t brightnessPercentage, PowerBacklightPercentage* backlight)
{
    Q_EMIT brightnessChanged(backlight, brightnessPercentage);
}

}  // namespace Kiran