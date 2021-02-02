/**
 * @file          /kiran-cc-daemon/plugins/power/backlight/power-backlight-monitor-tool.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
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
        LOG_DEBUG("run command: %s, exit code: %d.", cmdline.c_str(), exit_status);
        RETURN_VAL_IF_TRUE(exit_status != 0, false);
    }
    catch (const Glib::Error &e)
    {
        LOG_WARNING("%s.", e.what().c_str());
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
        LOG_DEBUG("run command: %s, exit code: %d.", cmdline.c_str(), exit_status);
        RETURN_VAL_IF_TRUE(exit_status != 0, -1);
        return std::strtol(standard_output.c_str(), nullptr, 0);
    }
    catch (const Glib::Error &e)
    {
        LOG_WARNING("%s.", e.what().c_str());
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
        LOG_DEBUG("run command: %s, exit code: %d.", cmdline.c_str(), exit_status);
        RETURN_VAL_IF_TRUE(exit_status != 0, false);
        max = std::strtol(standard_output.c_str(), nullptr, 0);
        LOG_DEBUG("min: %d, max: %d.", min, max);
    }
    catch (const Glib::Error &e)
    {
        LOG_WARNING("%s.", e.what().c_str());
        return false;
    }

    return true;
}

}  // namespace Kiran