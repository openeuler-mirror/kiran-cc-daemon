/**
 * @file          /kiran-cc-daemon/plugins/power/tools/power-backlight-helper.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#include "plugins/power/tools/power-backlight-helper.h"

#include <glib/gi18n.h>

#include "config.h"

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

PowerBacklightHelper::PowerBacklightHelper()
{
    this->backlight_dir_ = this->get_backlight_filepath();
}

PowerBacklightHelper::~PowerBacklightHelper()
{
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

}  // namespace  Kiran

int main(int argc, char* argv[])
{
    Kiran::PowerBacklightHelper backlight_helper;

    Gio::init();
    Kiran::Log::global_init();

    // TODO: zlog

    setlocale(LC_ALL, "");
    bindtextdomain(GETTEXT_PACKAGE, KCC_LOCALEDIR);
    bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
    textdomain(GETTEXT_PACKAGE);

    Glib::OptionContext context;
    Glib::OptionGroup group("backlight-helper", _("power backlight helper"));

    Glib::OptionEntry entry1;
    entry1.set_long_name("get-brightness-value");
    entry1.set_flags(Glib::OptionEntry::FLAG_NO_ARG);
    entry1.set_description(N_("Get the current brightness value"));

    Glib::OptionEntry entry2;
    entry2.set_long_name("get-max-brightness-value");
    entry2.set_flags(Glib::OptionEntry::FLAG_NO_ARG);
    entry2.set_description(N_("Get the max brightness value"));

    Glib::OptionEntry entry3;
    entry3.set_long_name("set-brightness-value");
    entry3.set_description(N_("Set the brightness value"));

    group.add_entry(entry1, [&backlight_helper](const Glib::ustring& option_name, const Glib::ustring&, bool) -> bool {
        auto brightness_value = backlight_helper.get_brightness_value();
        if (brightness_value >= 0)
        {
            fmt::print("{0}", brightness_value);
        }
        else
        {
            fmt::print("{0}", _("Could not get the value of the backlight"));
            return false;
        }
        return true;
    });

    group.add_entry(entry2, [&backlight_helper](const Glib::ustring& option_name, const Glib::ustring&, bool) -> bool {
        auto brightness_value = backlight_helper.get_brightness_max_value();
        if (brightness_value >= 0)
        {
            fmt::print("{0}", brightness_value);
        }
        else
        {
            fmt::print("{0}", _("Could not get the maximum value of the backlight"));
            return false;
        }
        return true;
    });

    group.add_entry(entry3, [&backlight_helper](const Glib::ustring& option_name, const Glib::ustring& value, bool has_value) -> bool {
        std::string error;
        auto brightness_value = std::strtol(value.c_str(), nullptr, 0);
        if (!backlight_helper.set_brightness_value(brightness_value, error))
        {
            fmt::print("{0}", error);
        }
        return true;
    });

    group.set_translation_domain(GETTEXT_PACKAGE);
    context.set_main_group(group);

    // 不支持获取和设置则直接返回
    if (!backlight_helper.support_backlight())
    {
        fmt::print("{0}", _("No backlights were found on your system"));
        return EXIT_FAILURE;
    }

    try
    {
        context.parse(argc, argv);
    }
    catch (const Glib::Exception& e)
    {
        LOG_WARNING("%s", e.what().c_str());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}