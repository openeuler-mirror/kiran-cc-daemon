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

#include "plugins/power/backlight/power-backlight-monitors-tool.h"
#include "config.h"
#include "plugins/power/backlight/power-backlight-monitor-tool.h"

namespace Kiran
{
#define POWER_BACKLIGHT_HELPER KCC_INSTALL_BINDIR "/kiran-power-backlight-helper"

PowerBacklightMonitorsTool::PowerBacklightMonitorsTool()
{
    auto backlight_dir = this->get_backlight_dir();
    if (!backlight_dir.empty())
    {
        auto filename = Glib::build_filename(backlight_dir, "brightness");
        this->brightness_monitor_ = FileUtils::make_monitor_file(filename,
                                                                 sigc::mem_fun(this, &PowerBacklightMonitorsTool::on_brightness_changed),
                                                                 Gio::FILE_MONITOR_NONE);
    }
}

bool PowerBacklightMonitorsTool::support_backlight()
{
    try
    {
        std::string standard_output;
        int32_t exit_status = 0;
        auto cmdline = fmt::format("pkexec {0} --support-backlight", POWER_BACKLIGHT_HELPER);
        Glib::spawn_command_line_sync(cmdline, &standard_output, nullptr, &exit_status);
        RETURN_VAL_IF_TRUE(exit_status != 0, false);
        return (std::strtol(standard_output.c_str(), nullptr, 0) == 1);
    }
    catch (const Glib::Error &e)
    {
        KLOG_WARNING("%s.", e.what().c_str());
    }
    return false;
}

void PowerBacklightMonitorsTool::init()
{
    this->backlight_monitors_.clear();
    this->backlight_monitors_.push_back(std::make_shared<PowerBacklightMonitorTool>());
}

std::string PowerBacklightMonitorsTool::get_backlight_dir()
{
    try
    {
        std::string standard_output;
        int32_t exit_status = 0;
        auto cmdline = fmt::format("pkexec {0} --get-backlight-dir", POWER_BACKLIGHT_HELPER);
        Glib::spawn_command_line_sync(cmdline, &standard_output, nullptr, &exit_status);
        RETURN_VAL_IF_TRUE(exit_status != 0, std::string());
        return standard_output;
    }
    catch (const Glib::Error &e)
    {
        KLOG_WARNING("%s.", e.what().c_str());
    }
    return std::string();
}

void PowerBacklightMonitorsTool::on_brightness_changed(const Glib::RefPtr<Gio::File> &file,
                                                       const Glib::RefPtr<Gio::File> &other_file,
                                                       Gio::FileMonitorEvent event_type)
{
    switch (event_type)
    {
    case Gio::FILE_MONITOR_EVENT_CHANGED:
    {
        this->brightness_changed_.emit();
        break;
    }
    default:
        break;
    }
}

}  // namespace Kiran