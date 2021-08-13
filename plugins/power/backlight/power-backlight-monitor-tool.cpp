/**
 * @Copyright (C) 2020 ~ 2021 KylinSec Co., Ltd. 
 *
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; If not, see <http: //www.gnu.org/licenses/>. 
 */

#include "plugins/power/backlight/power-backlight-monitor-tool.h"

#include "config.h"

namespace Kiran
{
#define POWER_BACKLIGHT_HELPER KCC_INSTALL_BINDIR "/kiran-power-backlight-helper"
PowerBacklightMonitorTool::PowerBacklightMonitorTool()
{
}

bool PowerBacklightMonitorTool::set_brightness_value(int32_t brightness_value)
{
    try
    {
        int32_t exit_status = 0;
        auto cmdline = fmt::format("pkexec {0} --set-brightness-value {1}", POWER_BACKLIGHT_HELPER, brightness_value);
        Glib::spawn_command_line_sync(cmdline, nullptr, nullptr, &exit_status);
        KLOG_DEBUG("run command: %s, exit code: %d.", cmdline.c_str(), exit_status);
        RETURN_VAL_IF_TRUE(exit_status != 0, false);
    }
    catch (const Glib::Error &e)
    {
        KLOG_WARNING("%s.", e.what().c_str());
        return false;
    }
    return true;
}

int32_t PowerBacklightMonitorTool::get_brightness_value()
{
    try
    {
        std::string standard_output;
        int32_t exit_status = 0;
        auto cmdline = fmt::format("{0} --get-brightness-value", POWER_BACKLIGHT_HELPER);
        Glib::spawn_command_line_sync(cmdline, &standard_output, nullptr, &exit_status);
        KLOG_DEBUG("run command: %s, exit code: %d.", cmdline.c_str(), exit_status);
        RETURN_VAL_IF_TRUE(exit_status != 0, -1);
        return std::strtol(standard_output.c_str(), nullptr, 0);
    }
    catch (const Glib::Error &e)
    {
        KLOG_WARNING("%s.", e.what().c_str());
    }
    return -1;
}

bool PowerBacklightMonitorTool::get_brightness_range(int32_t &min, int32_t &max)
{
    min = 0;
    max = 0;
    try
    {
        std::string standard_output;
        int32_t exit_status = 0;
        auto cmdline = fmt::format("{0} --get-max-brightness-value", POWER_BACKLIGHT_HELPER);
        Glib::spawn_command_line_sync(cmdline, &standard_output, nullptr, &exit_status);
        KLOG_DEBUG("run command: %s, exit code: %d.", cmdline.c_str(), exit_status);
        RETURN_VAL_IF_TRUE(exit_status != 0, false);
        max = std::strtol(standard_output.c_str(), nullptr, 0);
        KLOG_DEBUG("min: %d, max: %d.", min, max);
    }
    catch (const Glib::Error &e)
    {
        KLOG_WARNING("%s.", e.what().c_str());
        return false;
    }

    return true;
}

}  // namespace Kiran