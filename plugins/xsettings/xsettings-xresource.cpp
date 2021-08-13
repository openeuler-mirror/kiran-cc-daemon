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

#include "plugins/xsettings/xsettings-xresource.h"

#include "plugins/xsettings/xsettings-common.h"
#include "plugins/xsettings/xsettings-manager.h"
//
#include <X11/Xatom.h>

namespace Kiran
{
#define XRESOURCE_PROP_XFT_DPI "Xft.dpi"
#define XRESOURCE_PROP_XFT_ANTIALIAS "Xft.antialias"
#define XRESOURCE_PROP_XFT_HINTING "Xft.hinting"
#define XRESOURCE_PROP_XFT_HINTSTYLE "Xft.hintstyle"
#define XRESOURCE_PROP_XFT_RGBA "Xft.rgba"
#define XRESOURCE_PROP_XFT_LCDFILTER "Xft.lcdfilter"
#define XRESOURCE_PROP_XCURSOR_THEME "Xcursor.theme"
#define XRESOURCE_PROP_XCURSOR_SIZE "Xcursor.size"

XSettingsXResource::XSettingsXResource()
{
}

void XSettingsXResource::init()
{
    XSettingsManager::get_instance()->signal_xsettings_changed().connect(sigc::mem_fun(this, &XSettingsXResource::on_xsettings_changed));
}

void XSettingsXResource::on_xsettings_changed(const std::string &key)
{
    char dpibuf[G_ASCII_DTOSTR_BUF_SIZE];
    auto dpy = XOpenDisplay(NULL);
    auto xsettings_manager = XSettingsManager::get_instance();

    RETURN_IF_TRUE(dpy == NULL);
    RETURN_IF_TRUE(xsettings_manager == NULL);

    std::string props = XResourceManagerString(dpy);

    KLOG_DEBUG("Old Xresource: %s", props.c_str());
    auto xcursor_size = std::string(g_ascii_dtostr(dpibuf, sizeof(dpibuf), (double)xsettings_manager->get_gtk_cursor_theme_size()));

    switch (shash(key.c_str()))
    {
    case CONNECT(XSETTINGS_SCHEMA_WINDOW_SCALING_FACTOR, _hash):
    {
        auto dpi = std::string(g_ascii_dtostr(dpibuf, sizeof(dpibuf), (double)xsettings_manager->get_xft_dpi() / 1024.0));
        this->update_property(props, XRESOURCE_PROP_XFT_DPI, dpi);
        this->update_property(props, XRESOURCE_PROP_XCURSOR_SIZE, xcursor_size);
        break;
    }
    case CONNECT(XSETTINGS_SCHEMA_XFT_ANTIALIAS, _hash):
        this->update_property(props, XRESOURCE_PROP_XFT_ANTIALIAS, xsettings_manager->get_xft_antialias() > 0 ? "1" : "0");
        break;
    case CONNECT(XSETTINGS_SCHEMA_XFT_HINTING, _hash):
        this->update_property(props, XRESOURCE_PROP_XFT_HINTING, xsettings_manager->get_xft_hinting() > 0 ? "1" : "0");
        break;
    case CONNECT(XSETTINGS_SCHEMA_XFT_HINT_STYLE, _hash):
        this->update_property(props, XRESOURCE_PROP_XFT_HINTSTYLE, xsettings_manager->get_xft_hint_style());
        break;
    case CONNECT(XSETTINGS_SCHEMA_XFT_RGBA, _hash):
    {
        auto lcdfilter = (xsettings_manager->get_xft_rgba() == "rgb") ? "lcddefault" : "none";
        this->update_property(props, XRESOURCE_PROP_XFT_RGBA, xsettings_manager->get_xft_rgba());
        this->update_property(props, XRESOURCE_PROP_XFT_LCDFILTER, lcdfilter);
        break;
    }
    case CONNECT(XSETTINGS_SCHEMA_GTK_CURSOR_THEME_NAME, _hash):
        this->update_property(props, XRESOURCE_PROP_XCURSOR_THEME, xsettings_manager->get_gtk_cursor_theme_name());
        break;
    case CONNECT(XSETTINGS_SCHEMA_GTK_CURSOR_THEME_SIZE, _hash):
    {
        this->update_property(props, XRESOURCE_PROP_XCURSOR_SIZE, xcursor_size);
        break;
    }
    default:
        break;
    }

    KLOG_DEBUG("New Xresource: %s", props.c_str());

    XChangeProperty(dpy,
                    RootWindow(dpy, 0),
                    XA_RESOURCE_MANAGER,
                    XA_STRING,
                    8,
                    PropModeReplace,
                    (unsigned char *)props.c_str(),
                    props.length());
    XCloseDisplay(dpy);
}

void XSettingsXResource::update_property(std::string &props, const std::string &key, const std::string &value)
{
    auto needle = key + std::string(":");
    auto found_start = props.find(needle);

    // XrmValue数据格式 key:\tvalue\n
    if (found_start != std::string::npos)
    {
        auto found_end = props.find('\n', found_start);
        // +1是跳过制表符
        auto found_value = found_start + needle.length() + 1;
        if (found_end != std::string::npos)
        {
            props.erase(found_value, found_end - found_value + 1);
        }
        else
        {
            props.erase(found_value);
        }
        props.insert(found_value, 1, '\n');
        props.insert(found_value, value);
    }
    else
    {
        props += fmt::format("{0}:\t{1}\n", key, value);
    }
}
}  // namespace Kiran