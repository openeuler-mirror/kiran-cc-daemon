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
    this->update_properties();

    XSettingsManager::get_instance()->signal_xsettings_changed().connect(sigc::mem_fun(this, &XSettingsXResource::on_xsettings_changed));
}

void XSettingsXResource::update_properties()
{
    char dpibuf[G_ASCII_DTOSTR_BUF_SIZE];
    auto dpy = XOpenDisplay(NULL);
    auto xsettings_manager = XSettingsManager::get_instance();

    RETURN_IF_TRUE(dpy == NULL);
    RETURN_IF_TRUE(xsettings_manager == NULL);

    auto p_porps = XResourceManagerString(dpy);
    std::string props = POINTER_TO_STRING(p_porps);

    KLOG_DEBUG("Old Xresource: %s", props.c_str());
    auto xcursor_size = std::string(g_ascii_dtostr(dpibuf, sizeof(dpibuf), (double)xsettings_manager->get_gtk_cursor_theme_size()));

    auto dpi = std::string(g_ascii_dtostr(dpibuf, sizeof(dpibuf), (double)xsettings_manager->get_xft_dpi() / 1024.0));
    auto lcdfilter = (xsettings_manager->get_xft_rgba() == "rgb") ? "lcddefault" : "none";

    this->update_property(props, XRESOURCE_PROP_XFT_DPI, dpi);
    this->update_property(props, XRESOURCE_PROP_XCURSOR_SIZE, xcursor_size);
    this->update_property(props, XRESOURCE_PROP_XFT_ANTIALIAS, xsettings_manager->get_xft_antialias() > 0 ? "1" : "0");
    this->update_property(props, XRESOURCE_PROP_XFT_HINTING, xsettings_manager->get_xft_hinting() > 0 ? "1" : "0");
    this->update_property(props, XRESOURCE_PROP_XFT_HINTSTYLE, xsettings_manager->get_xft_hint_style());
    this->update_property(props, XRESOURCE_PROP_XFT_RGBA, xsettings_manager->get_xft_rgba());
    this->update_property(props, XRESOURCE_PROP_XFT_LCDFILTER, lcdfilter);
    this->update_property(props, XRESOURCE_PROP_XCURSOR_THEME, xsettings_manager->get_gtk_cursor_theme_name());
    this->update_property(props, XRESOURCE_PROP_XCURSOR_SIZE, xcursor_size);

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

void XSettingsXResource::on_xsettings_changed(const std::string &key)
{
    switch (shash(key.c_str()))
    {
    case CONNECT(XSETTINGS_SCHEMA_WINDOW_SCALING_FACTOR, _hash):
    case CONNECT(XSETTINGS_SCHEMA_XFT_ANTIALIAS, _hash):
    case CONNECT(XSETTINGS_SCHEMA_XFT_HINTING, _hash):
    case CONNECT(XSETTINGS_SCHEMA_XFT_HINT_STYLE, _hash):
    case CONNECT(XSETTINGS_SCHEMA_XFT_RGBA, _hash):
    case CONNECT(XSETTINGS_SCHEMA_XFT_DPI, _hash):
    case CONNECT(XSETTINGS_SCHEMA_GTK_CURSOR_THEME_NAME, _hash):
    case CONNECT(XSETTINGS_SCHEMA_GTK_CURSOR_THEME_SIZE, _hash):
        this->update_properties();
        break;
    default:
        break;
    }
}
}  // namespace Kiran
