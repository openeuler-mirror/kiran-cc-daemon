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

#include "plugins/power/power-utils.h"

#include <fmt/format.h>
#include <glib/gi18n.h>
#include <power-i.h>

namespace Kiran
{
std::string PowerUtils::get_time_translation(uint32_t seconds)
{
    auto minutes = seconds / 60;

    RETURN_VAL_IF_TRUE(minutes == 0, _("Less than 1 minute"));

    if (minutes < 60)
    {
        return fmt::format(ngettext("{0} minute", "{0} minutes", minutes), minutes);
    }

    auto hours = minutes / 60;
    minutes = minutes % 60;

    if (minutes == 0)
    {
        return fmt::format(ngettext("{0} hour", "{0} hours", hours), hours);
    }
    else
    {
        return fmt::format("{0} {1} {2} {3}",
                           hours,
                           ngettext("hour", "hours", hours),
                           minutes,
                           ngettext("minute", "minutes", minutes));
    }
}

std::string PowerUtils::action_enum2str(uint32_t action)
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

std::string PowerUtils::event_enum2str(uint32_t event)
{
    switch (event)
    {
    case PowerEvent::POWER_EVENT_PRESSED_POWEROFF:
        return "power off pressed";
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
    case PowerEvent::POWER_EVENT_PRESSED_BATTERY:
        return "battery pressed";
    case PowerEvent::POWER_EVENT_BATTERY_CHARGE_ACTION:
        return "battery charge action";
    default:
        return "unknown";
    }
}

std::string PowerUtils::device_enum2str(uint32_t device)
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

std::string PowerUtils::supply_enum2str(uint32_t supply)
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