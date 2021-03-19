/**
 * @file          /kiran-cc-daemon/plugins/power/tools/power-backlight-helper.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#include "plugins/power/tools/power-backlight-helper.h"

#include <glib/gi18n.h>

#include "lib/base/base.h"

namespace Kiran
{
#define POWER_BACKLIGHT_SYS_PATH "/sys/class/backlight"

const std::vector<std::string> PowerBacklightHelper::backlight_search_subdirs_ = {
    "gmux_backlight",
    "nv_backlight",
    "nvidia_backlight",
    "intel_backlight",
    "dell_backlight",
    "asus_laptop",
    "toshiba",
    "eeepc",
    "eeepc-wmi",
    "thinkpad_screen",
    "acpi_video1",
    "mbp_backlight",
    "acpi_video0",
    "fujitsu-laptop",
    "sony",
    "samsung"};

PowerBacklightHelper::PowerBacklightHelper() : brightness_value_(-1)
{
    this->backlight_dir_ = this->get_backlight_filepath();
}

PowerBacklightHelper::~PowerBacklightHelper()
{
}

void PowerBacklightHelper::init()
{
    if (this->backlight_dir_.length() > 0)
    {
        auto filename = Glib::build_filename(this->backlight_dir_, "brightness");
        this->brightness_monitor_ = FileUtils::make_monitor_file(filename, sigc::mem_fun(this, &PowerBacklightHelper::on_brightness_changed), Gio::FILE_MONITOR_NONE);
        this->brightness_value_ = this->get_brightness_value();
    }
}

int32_t PowerBacklightHelper::get_brightness_value()
{
    RETURN_VAL_IF_FALSE(this->backlight_dir_.length() > 0, -1);

    try
    {
        auto filename = Glib::build_filename(this->backlight_dir_, "brightness");
        auto content = Glib::file_get_contents(filename);
        return (int32_t)std::strtol(content.c_str(), nullptr, 0);
    }
    catch (const Glib::Error& e)
    {
        LOG_WARNING("%s.", e.what().c_str());
    }
    return -1;
}

int32_t PowerBacklightHelper::get_brightness_max_value()
{
    RETURN_VAL_IF_FALSE(this->backlight_dir_.length() > 0, -1);

    try
    {
        auto filename = Glib::build_filename(this->backlight_dir_, "max_brightness");
        auto content = Glib::file_get_contents(filename);
        return (int32_t)std::strtol(content.c_str(), nullptr, 0);
    }
    catch (const Glib::Error& e)
    {
        LOG_WARNING("%s.", e.what().c_str());
    }
    return -1;
}

bool PowerBacklightHelper::set_brightness_value(int32_t brightness_value, std::string& error)
{
    auto uid = getuid();
    auto euid = geteuid();
    if (uid != 0 || euid != 0)
    {
        error = fmt::format("{0}", _("This program can only be used by the root user"));
        return false;
    }

    auto pkexec_uid_str = g_getenv("PKEXEC_UID");
    if (pkexec_uid_str == NULL)
    {
        error = fmt::format("{0}", _("This program must only be run through pkexec"));
        return false;
    }

    auto contents = fmt::format("{0}", brightness_value);
    auto filename = Glib::build_filename(this->backlight_dir_, "brightness");

    if (!FileUtils::write_contents(filename, contents))
    {
        error = fmt::format("{0}", _("Could not set the value of the backlight"));
        return false;
    }
    return true;
}

std::string PowerBacklightHelper::get_backlight_filepath()
{
    // 先搜索指定的目录
    for (auto& sub_dir : this->backlight_search_subdirs_)
    {
        auto backlight_dir = Glib::build_filename(POWER_BACKLIGHT_SYS_PATH, sub_dir);
        if (Glib::file_test(backlight_dir, Glib::FileTest::FILE_TEST_EXISTS))
        {
            return backlight_dir;
        }
    }

    // 搜索不到的情况下选择第一个目录
    try
    {
        Glib::Dir dir(POWER_BACKLIGHT_SYS_PATH);

        auto subdir = dir.read_name();
        if (subdir.size() > 0)
        {
            return Glib::build_filename(POWER_BACKLIGHT_SYS_PATH, subdir);
        }
    }
    catch (const Glib::Error& e)
    {
        LOG_WARNING("%s.", e.what().c_str());
        return std::string();
    }
    return std::string();
}

void PowerBacklightHelper::on_brightness_changed(const Glib::RefPtr<Gio::File>& file,
                                                 const Glib::RefPtr<Gio::File>& other_file,
                                                 Gio::FileMonitorEvent event_type)
{
    switch (event_type)
    {
    case Gio::FILE_MONITOR_EVENT_CHANGED:
    {
        auto brightness_value = this->get_brightness_value();
        if (brightness_value != this->brightness_value_)
        {
            this->brightness_value_ = brightness_value;
            this->brightness_changed_.emit(this->brightness_value_);
        }
        break;
    }
    default:
        break;
    }
}

}  // namespace  Kiran
