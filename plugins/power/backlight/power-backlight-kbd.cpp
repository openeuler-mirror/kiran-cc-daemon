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

#include "power-backlight-kbd.h"
#include <QDBusConnection>
#include <QDBusMessage>

namespace Kiran
{
#define UPOWER_KBD_BACKLIGHT_DBUS_NAME "org.freedesktop.UPower"
#define UPOWER_KBD_BACKLIGHT_DBUS_OBJECT "/org/freedesktop/UPower/KbdBacklight"
#define UPOWER_KBD_BACKLIGHT_DBUS_INTERFACE "org.freedesktop.UPower.KbdBacklight"

#define POWER_KBD_BACKLIGHT_STEP 10

PowerBacklightKbd::PowerBacklightKbd(QObject* parent) : PowerBacklightPercentage(parent),
                                                        m_brightnessValue(-1),
                                                        m_brightnessPercentage(-1),
                                                        m_maxBrightnessValue(-1)
{
}

PowerBacklightKbd::~PowerBacklightKbd()
{
}

void PowerBacklightKbd::init()
{
    m_maxBrightnessValue = getMaxBrightnessValue();
    // 判断是否支持亮度设置
    RETURN_IF_TRUE(m_maxBrightnessValue <= 1);

    m_brightnessValue = getBrightnessValue();
    m_brightnessPercentage = brightnessDiscrete2Percent(m_brightnessValue, m_maxBrightnessValue);

    QDBusConnection::systemBus().connect(UPOWER_KBD_BACKLIGHT_DBUS_NAME,
                                         UPOWER_KBD_BACKLIGHT_DBUS_OBJECT,
                                         UPOWER_KBD_BACKLIGHT_DBUS_INTERFACE,
                                         "BrightnessChanged",
                                         this,
                                         SLOT(processBrightnessChanged(const QDBusMessage&)));
}

bool PowerBacklightKbd::setBrightness(int32_t percentage)
{
    RETURN_VAL_IF_TRUE(m_maxBrightnessValue <= 1, false);
    RETURN_VAL_IF_TRUE(percentage == m_brightnessPercentage, true);

    auto newBrightnessValue = brightnessPercent2Discrete(percentage, m_maxBrightnessValue);
    auto adjustScale = (percentage > m_brightnessPercentage) ? 1 : -1;

    // 如果设置的百分比和当前的百分比对应的值是相同的，则向上或者向下调整亮度值
    if (newBrightnessValue == m_brightnessValue)
    {
        newBrightnessValue += adjustScale;
    }

    while (m_brightnessValue != newBrightnessValue)
    {
        m_brightnessValue += adjustScale;
        if (!setBrightnessValue(m_brightnessValue))
        {
            break;
        }
    }
    m_brightnessPercentage = brightnessDiscrete2Percent(m_brightnessValue, m_maxBrightnessValue);

    KLOG_INFO(power) << "Current brightness is" << m_brightnessValue << ", set" << newBrightnessValue << "to be new brightness.";
    return (m_brightnessValue == newBrightnessValue);
}

bool PowerBacklightKbd::brightnessUp()
{
    RETURN_VAL_IF_TRUE(m_maxBrightnessValue <= 1, false);

    auto brightnessPercentage = std::min(m_brightnessPercentage + POWER_KBD_BACKLIGHT_STEP, 100);
    return setBrightness(brightnessPercentage);
}

bool PowerBacklightKbd::brightnessDown()
{
    RETURN_VAL_IF_TRUE(m_maxBrightnessValue <= 1, false);

    auto brightnessPercentage = std::max(m_brightnessPercentage - POWER_KBD_BACKLIGHT_STEP, 0);
    return setBrightness(brightnessPercentage);
}

int32_t PowerBacklightKbd::getBrightnessValue()
{
    auto sendMessage = QDBusMessage::createMethodCall(UPOWER_KBD_BACKLIGHT_DBUS_NAME,
                                                      UPOWER_KBD_BACKLIGHT_DBUS_OBJECT,
                                                      UPOWER_KBD_BACKLIGHT_DBUS_INTERFACE,
                                                      "GetBrightness");

    auto replyMessage = QDBusConnection::systemBus().call(sendMessage, QDBus::Block);
    if (replyMessage.type() == QDBusMessage::ErrorMessage)
    {
        KLOG_INFO(power) << "Call GetBrightness return error:" << replyMessage.errorMessage();
        return -1;
    }
    else
    {
        return replyMessage.arguments().takeFirst().value<int>();
    }
}

bool PowerBacklightKbd::setBrightnessValue(int32_t value)
{
    auto sendMessage = QDBusMessage::createMethodCall(UPOWER_KBD_BACKLIGHT_DBUS_NAME,
                                                      UPOWER_KBD_BACKLIGHT_DBUS_OBJECT,
                                                      UPOWER_KBD_BACKLIGHT_DBUS_INTERFACE,
                                                      "SetBrightness");

    sendMessage << value;

    auto replyMessage = QDBusConnection::systemBus().call(sendMessage, QDBus::Block);
    if (replyMessage.type() == QDBusMessage::ErrorMessage)
    {
        KLOG_INFO(power) << "Call SetBrightness return error:" << replyMessage.errorMessage();
        return false;
    }
    return true;
}

int32_t PowerBacklightKbd::getMaxBrightnessValue()
{
    auto sendMessage = QDBusMessage::createMethodCall(UPOWER_KBD_BACKLIGHT_DBUS_NAME,
                                                      UPOWER_KBD_BACKLIGHT_DBUS_OBJECT,
                                                      UPOWER_KBD_BACKLIGHT_DBUS_INTERFACE,
                                                      "GetMaxBrightness");

    auto replyMessage = QDBusConnection::systemBus().call(sendMessage, QDBus::Block);
    if (replyMessage.type() == QDBusMessage::ErrorMessage)
    {
        KLOG_INFO(power) << "Call GetMaxBrightness return error:" << replyMessage.errorMessage();
        return -1;
    }
    else
    {
        return replyMessage.arguments().takeFirst().value<int>();
    }
}

int32_t PowerBacklightKbd::brightnessPercent2Discrete(int32_t percentage, int32_t levels)
{
    RETURN_VAL_IF_TRUE(percentage > 100, levels);
    RETURN_VAL_IF_TRUE(levels == 0, 0);

    /* for levels < 10 min value is 0 */
    int32_t factor = levels < 10 ? 0 : 1;
    return (int32_t)((((double)percentage * (double)(levels - factor)) / 100.0f) + 0.5f);
}

int32_t PowerBacklightKbd::brightnessDiscrete2Percent(int32_t discrete, int32_t levels)
{
    RETURN_VAL_IF_TRUE(discrete > levels, 100);
    RETURN_VAL_IF_TRUE(levels == 0, 0);

    /* for levels < 10 min value is 0 */
    int32_t factor = levels < 10 ? 0 : 1;
    return (int32_t)(((double)discrete * (100.0f / (double)(levels - factor))) + 0.5f);
}

void PowerBacklightKbd::processBrightnessChanged(const QDBusMessage& message)
{
    m_brightnessValue = message.arguments().takeFirst().value<int>();
    m_brightnessPercentage = brightnessDiscrete2Percent(m_brightnessValue, m_maxBrightnessValue);
    Q_EMIT brightnessChanged(m_brightnessPercentage);
}

}  // namespace  Kiran
