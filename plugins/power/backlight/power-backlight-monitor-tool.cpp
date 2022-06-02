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