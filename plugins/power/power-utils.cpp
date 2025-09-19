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

#include "power-utils.h"
#include <power-i.h>
#include "lib/base/base.h"

namespace Kiran
{
QString PowerUtils::getTimeTranslation(uint32_t seconds)
{
    auto minutes = seconds / 60;

    RETURN_VAL_IF_TRUE(minutes == 0, tr("Less than 1 minute"));

    if (minutes < 60)
    {
        auto minutesTranslation = (minutes <= 1) ? tr("%1 minute") : tr("%1 minutes");
        return QString(minutesTranslation).arg(minutes);
    }

    auto hours = minutes / 60;
    minutes = minutes % 60;

    auto minutesTranslation = (minutes <= 1) ? tr("%1 minute") : tr("%1 minutes");
    minutesTranslation = QString(minutesTranslation).arg(minutes);

    auto hoursTranslation = (hours <= 1) ? tr("%1 hour") : tr("%1 hours");
    hoursTranslation = QString(hoursTranslation).arg(hours);

    if (minutes == 0)
    {
        return hoursTranslation;
    }
    else
    {
        return QString("%1 %2").arg(hoursTranslation).arg(minutesTranslation);
    }
}

QString PowerUtils::actionEnum2str(uint32_t action)
{
    switch (action)
    {
    case PowerAction::POWER_ACTION_DISPLAY_ON:
        return "display on";
    case PowerAction::POWER_ACTION_DISPLAY_STANDBY:
        return "display standby";
    case PowerAction::POWER_ACTION_DISPLAY_SUSPEND:
        return "display suspend";
    case PowerAction::POWER_ACTION_DISPLAY_OFF:
        return "display off";
    case PowerAction::POWER_ACTION_COMPUTER_SUSPEND:
        return "computer suspend";
    case PowerAction::POWER_ACTION_COMPUTER_SHUTDOWN:
        return "computer shutdown";
    case PowerAction::POWER_ACTION_COMPUTER_HIBERNATE:
        return "computer hibernate";
    case PowerAction::POWER_ACTION_NOTHING:
        return "nothing";
    default:
        return "unknown";
    };
}

QString PowerUtils::eventActionEnum2Str(int eventAction)
{
    switch (eventAction)
    {
    case POWER_ACTION_DISPLAY_OFF:
        return "blank";
    case POWER_ACTION_COMPUTER_SUSPEND:
        return "suspend";
    case POWER_ACTION_COMPUTER_SHUTDOWN:
        return "shutdown";
    case POWER_ACTION_COMPUTER_HIBERNATE:
        return "hibernate";
    default:
        break;
    }
    return "nothing";
}

int PowerUtils::eventActionStr2Enum(QString eventActionStr)
{
    switch (shash(eventActionStr.toUtf8().data()))
    {
    case "blank"_hash:
        return POWER_ACTION_DISPLAY_OFF;
    case "suspend"_hash:
        return POWER_ACTION_COMPUTER_SUSPEND;
    case "shutdown"_hash:
        return POWER_ACTION_COMPUTER_SHUTDOWN;
    case "hibernate"_hash:
        return POWER_ACTION_COMPUTER_HIBERNATE;
    default:
        break;
    }
    return POWER_ACTION_NOTHING;
}

QString PowerUtils::monitorActionEnum2Str(int monitorAction)
{
    switch (monitorAction)
    {
    case POWER_ACTION_DISPLAY_STANDBY:
        return "standby";
    case POWER_ACTION_DISPLAY_SUSPEND:
        return "suspend";
    case POWER_ACTION_DISPLAY_OFF:
        return "off";
    default:
        break;
    }
    return "nothing";
}

int PowerUtils::monitorActionStr2Enum(QString monitorActionStr)
{
    switch (shash(monitorActionStr.toUtf8().data()))
    {
    case "standby"_hash:
        return POWER_ACTION_DISPLAY_STANDBY;
    case "suspend"_hash:
        return POWER_ACTION_DISPLAY_SUSPEND;
    case "off"_hash:
        return POWER_ACTION_DISPLAY_OFF;
    default:
        break;
    }
    return POWER_ACTION_NOTHING;
}

QString PowerUtils::computerActionEnum2Str(int computerAction)
{
    switch (computerAction)
    {
    case POWER_ACTION_COMPUTER_SUSPEND:
        return "suspend";
    case POWER_ACTION_COMPUTER_SHUTDOWN:
        return "shutdown";
    case POWER_ACTION_COMPUTER_HIBERNATE:
        return "hibernate";
    default:
        break;
    }
    return "nothing";
}

int PowerUtils::computerActionStr2Enum(QString computerActionStr)
{
    switch (shash(computerActionStr.toUtf8().data()))
    {
    case "suspend"_hash:
        return POWER_ACTION_COMPUTER_SUSPEND;
    case "shutdown"_hash:
        return POWER_ACTION_COMPUTER_SHUTDOWN;
    case "hibernate"_hash:
        return POWER_ACTION_COMPUTER_HIBERNATE;
    default:
        break;
    }
    return POWER_ACTION_NOTHING;
}

QString PowerUtils::eventEnum2Str(uint32_t event)
{
    switch (event)
    {
    case PowerEvent::POWER_EVENT_RELEASE_POWEROFF:
        return "power off release";
    case PowerEvent::POWER_EVENT_PRESSED_SLEEP:
        return "sleep pressed";
    case PowerEvent::POWER_EVENT_PRESSED_SUSPEND:
        return "suspend pressed";
    case PowerEvent::POWER_EVENT_PRESSED_HIBERNATE:
        return "hibernate pressed";
    case PowerEvent::POWER_EVENT_LID_OPEN:
        return "lid opened";
    case PowerEvent::POWER_EVENT_LID_CLOSED:
        return "lid closed";
    case PowerEvent::POWER_EVENT_PRESSED_BRIGHT_UP:
        return "bright up pressed";
    case PowerEvent::POWER_EVENT_PRESSED_KBD_BRIGHT_DOWN:
        return "kbd bright down pressed";
    case PowerEvent::POWER_EVENT_PRESSED_KBD_BRIGHT_TOGGLE:
        return "kbd bright toggle pressed";
    case PowerEvent::POWER_EVENT_PRESSED_LOCK:
        return "lock pressed";
    case PowerEvent::POWER_EVENT_BATTERY_CHARGE_ACTION:
        return "battery charge action";
    default:
        return "unknown";
    }
}

QString PowerUtils::deviceEnum2Str(uint32_t device)
{
    switch (device)
    {
    case PowerDeviceType::POWER_DEVICE_TYPE_COMPUTER:
        return "computer";
    case PowerDeviceType::POWER_DEVICE_TYPE_MONITOR:
        return "monitor";
    case PowerDeviceType::POWER_DEVICE_TYPE_KBD:
        return "keyboard";
    case PowerDeviceType::POWER_DEVICE_TYPE_BACKLIGHT:
        return "backlight";
    default:
        return "unknown";
    }
}

QString PowerUtils::supplyEnum2Str(uint32_t supply)
{
    switch (supply)
    {
    case PowerSupplyMode::POWER_SUPPLY_MODE_BATTERY:
        return "battery";
    case PowerSupplyMode::POWER_SUPPLY_MODE_AC:
        return "ac adapter";
    case PowerSupplyMode::POWER_SUPPLY_MODE_UPS:
        return "ups";
    default:
        return "unknown";
    }
}

}  // namespace Kiran