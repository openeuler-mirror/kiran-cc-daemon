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

#include "plugins/xsettings/xsettings-utils.h"

#include <gdk/gdk.h>
#include <gdk/gdkwayland.h>
#include <gdk/gdkx.h>
//
#include <X11/Xatom.h>

#include "plugins/xsettings/xsettings-common.h"

// 这里大部分功能直接沿用mate xsettings插件代码

namespace Kiran
{
double XSettingsUtils::get_dpi_from_x_server()
{
    GdkScreen *screen = NULL;
    double dpi;

    screen = gdk_screen_get_default();
    if (screen != NULL)
    {
        double width_dpi;
        double height_dpi;

        Screen *xscreen = gdk_x11_screen_get_xscreen(screen);

        width_dpi = XSettingsUtils::dpi_from_pixels_and_mm(WidthOfScreen(xscreen), WidthMMOfScreen(xscreen));
        height_dpi = XSettingsUtils::dpi_from_pixels_and_mm(HeightOfScreen(xscreen), HeightMMOfScreen(xscreen));

        if (width_dpi < DPI_LOW_REASONABLE_VALUE ||
            width_dpi > DPI_HIGH_REASONABLE_VALUE ||
            height_dpi < DPI_LOW_REASONABLE_VALUE ||
            height_dpi > DPI_HIGH_REASONABLE_VALUE)
        {
            dpi = DPI_FALLBACK;
        }
        else
        {
            dpi = (width_dpi + height_dpi) / 2.0;
        }
    }
    else
    {
        /* Huh!?  No screen? */
        dpi = DPI_FALLBACK;
    }

    return dpi;
}

/* Auto-detect the most appropriate scale factor for the primary monitor.
 * A lot of this code is shamelessly copied and adapted from Linux Mint/Cinnamon.
 */
int XSettingsUtils::get_window_scale_auto()
{
    GdkDisplay *display;
    GdkMonitor *monitor;
    GdkRectangle rect;
    int width_mm, height_mm;
    int monitor_scale, window_scale;

    display = gdk_display_get_default();
    monitor = gdk_display_get_primary_monitor(display);

    /* Use current value as the default */
    window_scale = 1;

    gdk_monitor_get_geometry(monitor, &rect);
    width_mm = gdk_monitor_get_width_mm(monitor);
    height_mm = gdk_monitor_get_height_mm(monitor);
    monitor_scale = gdk_monitor_get_scale_factor(monitor);

    if (rect.height * monitor_scale < HIDPI_MIN_HEIGHT)
        return 1;

    /* Some monitors/TV encode the aspect ratio (16/9 or 16/10) instead of the physical size */
    if ((width_mm == 160 && height_mm == 90) ||
        (width_mm == 160 && height_mm == 100) ||
        (width_mm == 16 && height_mm == 9) ||
        (width_mm == 16 && height_mm == 10))
        return 1;

    if (width_mm > 0 && height_mm > 0)
    {
        double dpi_x, dpi_y;

        dpi_x = (double)rect.width * monitor_scale / (width_mm / 25.4);
        dpi_y = (double)rect.height * monitor_scale / (height_mm / 25.4);
        /* We don't completely trust these values so both must be high, and never pick
                 * higher ratio than 2 automatically */
        if (dpi_x > HIDPI_LIMIT && dpi_y > HIDPI_LIMIT)
            window_scale = 2;
    }

    return window_scale;
}

bool XSettingsUtils::update_user_env_variable(const std::string &variable,
                                              const std::string &value,
                                              std::string &error)
{
    try
    {
        auto connection = Gio::DBus::Connection::get_sync(Gio::DBus::BUS_TYPE_SESSION);
        connection->call_sync("/org/gnome/SessionManager",
                              "org.gnome.SessionManager",
                              "Setenv",
                              Glib::VariantContainerBase(g_variant_new("(ss)", variable.c_str(), value.c_str())),
                              "org.gnome.SessionManager");
    }
    catch (const Glib::Error &e)
    {
        error = e.what().raw();
        return false;
    }
    return true;
}

double XSettingsUtils::dpi_from_pixels_and_mm(int pixels, int mm)
{
    double dpi;

    if (mm >= 1)
        dpi = pixels / (mm / 25.4);
    else
        dpi = 0;

    return dpi;
}

double XSettingsUtils::format_scale_dpi(int scale, double dpi)
{
    return double(CLAMP(dpi * scale, DPI_LOW_REASONABLE_VALUE, DPI_HIGH_REASONABLE_VALUE));
}
}  // namespace  Kiran
