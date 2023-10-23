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

#include <glib/gi18n.h>
#include <gtk3-log-i.h>

#include "config.h"
#include "plugins/power/tools/power-backlight-helper.h"

struct CommandOptions
{
    CommandOptions() : show_version(false),
                       support_backlight(false),
                       get_backlight_direcotry(false),
                       get_brightness_value(false),
                       get_max_brightness_value(false),
                       set_brightness_value(-1) {}
    bool show_version;
    bool support_backlight;
    bool get_backlight_direcotry;
    bool get_brightness_value;
    bool get_max_brightness_value;
    int32_t set_brightness_value;
};

int main(int argc, char* argv[])
{
    CommandOptions options;
    Kiran::PowerBacklightHelper backlight_helper;

    Gio::init();
    backlight_helper.init();

    klog_gtk3_init(std::string(), "kylinsec-system", "kiran-cc-daemon", "kiran-power-backlight-helper");

    setlocale(LC_ALL, "");
    bindtextdomain(GETTEXT_PACKAGE, KCC_LOCALEDIR);
    bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
    textdomain(GETTEXT_PACKAGE);

    Glib::OptionContext context;
    Glib::OptionGroup group("backlight-helper", _("power backlight helper"));

    group.add_entry(Kiran::MiscUtils::create_option_entry("version", N_("Output version infomation and exit.")),
                    options.show_version);
    group.add_entry(Kiran::MiscUtils::create_option_entry("support-backlight", N_("Whether the backlight device exists.")),
                    options.support_backlight);
    group.add_entry(Kiran::MiscUtils::create_option_entry("get-backlight-directory", N_("Get backlight monitor directory.")),
                    options.support_backlight);
    group.add_entry(Kiran::MiscUtils::create_option_entry("get-brightness-value", N_("Get the current brightness value.")),
                    options.get_brightness_value);
    group.add_entry(Kiran::MiscUtils::create_option_entry("get-max-brightness-value", N_("Get the max brightness value.")),
                    options.get_max_brightness_value);
    group.add_entry(Kiran::MiscUtils::create_option_entry("set-brightness-value", N_("Set the brightness value.")),
                    options.set_brightness_value);

    group.set_translation_domain(GETTEXT_PACKAGE);
    context.set_main_group(group);

    try
    {
        context.parse(argc, argv);
    }
    catch (const Glib::Exception& e)
    {
        KLOG_WARNING_POWER("%s", e.what().c_str());
        return EXIT_FAILURE;
    }

    if (options.show_version)
    {
        fmt::print("{0}", PROJECT_VERSION);
        return EXIT_SUCCESS;
    }

    if (options.support_backlight)
    {
        fmt::print("{0}", backlight_helper.support_backlight() ? 1 : 0);
        return EXIT_SUCCESS;
    }

    if (options.get_backlight_direcotry)
    {
        fmt::print("{0}", backlight_helper.get_backlight_dir());
        return EXIT_SUCCESS;
    }

    // 不支持获取和设置则直接返回
    if (!backlight_helper.support_backlight())
    {
        fmt::print(stderr, "{0}", _("No backlights were found on your system"));
        return EXIT_FAILURE;
    }

    if (options.get_brightness_value)
    {
        auto brightness_value = backlight_helper.get_brightness_value();
        if (brightness_value >= 0)
        {
            fmt::print("{0}", brightness_value);
        }
        else
        {
            fmt::print(stderr, "{0}", _("Could not get the value of the backlight"));
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }

    if (options.get_max_brightness_value)
    {
        auto brightness_value = backlight_helper.get_brightness_max_value();
        if (brightness_value >= 0)
        {
            fmt::print("{0}", brightness_value);
        }
        else
        {
            fmt::print(stderr, "{0}", _("Could not get the maximum value of the backlight"));
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }

    if (options.set_brightness_value >= 0)
    {
        std::string error;
        if (!backlight_helper.set_brightness_value(options.set_brightness_value, error))
        {
            fmt::print(stderr, "{0}", error);
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }

    return EXIT_SUCCESS;
}