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

#include "settings-xresource.h"
#include <glib.h>
#include <xcb/xcb.h>
#include <QGSettings>
#include "lib/base/base.h"
#include "lib/xcb/xcb-connection.h"
#include "settings-common.h"
#include "settings-manager.h"

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

SettingsXResource::SettingsXResource(QObject *parent) : QObject(parent)
{
    m_xcbConnection = XcbConnection::getDefault();
    m_settings = new QGSettings(SETTINGS_SCHEMA_ID, "", this);
}

void SettingsXResource::init()
{
    updateProperties();

    connect(m_settings, &QGSettings::changed, this, &SettingsXResource::processXsettingsChanged);
}

void SettingsXResource::updateProperties()
{
    char dpibuf[G_ASCII_DTOSTR_BUF_SIZE];
    auto settingsManager = SettingsManager::getInstance();
    auto xcbConnection = m_xcbConnection->getConnection();
    auto rootWindow = m_xcbConnection->getDefaultScreen()->root;

    RETURN_IF_TRUE(settingsManager == NULL);

    int offset = 0;
    QByteArray resourceProperties;
    while (true)
    {
        auto reply = XCB_REPLY_UNCHECKED(xcb_get_property,
                                         xcbConnection,
                                         false,
                                         rootWindow,
                                         XCB_ATOM_RESOURCE_MANAGER,
                                         XCB_ATOM_STRING, offset / 4, 8192);
        bool more = false;
        if (reply && reply->format == 8 && reply->type == XCB_ATOM_STRING)
        {
            resourceProperties += QByteArray(static_cast<const char *>(xcb_get_property_value(reply.get())), xcb_get_property_value_length(reply.get()));
            offset += xcb_get_property_value_length(reply.get());
            more = reply->bytes_after != 0;
        }

        BREAK_IF_TRUE(!more)
    }

    KLOG_INFO(settings) << "The Old Xresource is" << resourceProperties;
    auto xcursorSize = QString(g_ascii_dtostr(dpibuf, sizeof(dpibuf), (double)settingsManager->getCursorThemeSize()));

    auto dpi = QString(g_ascii_dtostr(dpibuf, sizeof(dpibuf), (double)settingsManager->getXftDPI() / 1024.0));
    auto lcdfilter = (settingsManager->getXftRGBA() == "rgb") ? "lcddefault" : "none";

    updateProperty(resourceProperties, XRESOURCE_PROP_XFT_DPI, dpi);
    updateProperty(resourceProperties, XRESOURCE_PROP_XCURSOR_SIZE, xcursorSize);
    updateProperty(resourceProperties, XRESOURCE_PROP_XFT_ANTIALIAS, settingsManager->getXftAntialias() > 0 ? "1" : "0");
    updateProperty(resourceProperties, XRESOURCE_PROP_XFT_HINTING, settingsManager->getXftHinting() > 0 ? "1" : "0");
    updateProperty(resourceProperties, XRESOURCE_PROP_XFT_HINTSTYLE, settingsManager->getXftHintStyle());
    updateProperty(resourceProperties, XRESOURCE_PROP_XFT_RGBA, settingsManager->getXftRGBA());
    updateProperty(resourceProperties, XRESOURCE_PROP_XFT_LCDFILTER, lcdfilter);
    updateProperty(resourceProperties, XRESOURCE_PROP_XCURSOR_THEME, settingsManager->getCursorThemeName());
    updateProperty(resourceProperties, XRESOURCE_PROP_XCURSOR_SIZE, xcursorSize);

    KLOG_INFO(settings) << "The New Xresource is" << resourceProperties;

    xcb_change_property(xcbConnection,
                        XCB_PROP_MODE_REPLACE,
                        rootWindow,
                        XCB_ATOM_RESOURCE_MANAGER,
                        XCB_ATOM_STRING, 8, resourceProperties.length(),
                        resourceProperties.data());

    xcb_flush(m_xcbConnection->getConnection());
}

void SettingsXResource::updateProperty(QByteArray &props, const QString &key, const QString &value)
{
    auto needle = key + QString(":");
    auto foundStart = props.indexOf(needle.toUtf8());

    // XrmValue数据格式 key:\tvalue\n
    if (foundStart >= 0)
    {
        auto foundEnd = props.indexOf('\n', foundStart);
        // +1是跳过制表符
        auto foundValue = foundStart + needle.length() + 1;
        if (foundEnd >= 0)
        {
            props.remove(foundValue, foundEnd - foundValue + 1);
        }
        else
        {
            props.remove(foundValue, props.length() - foundValue);
        }
        props.insert(foundValue, 1, '\n');
        props.insert(foundValue, value.toUtf8());
    }
    else
    {
        props += QString("%1:\t%2\n").arg(key).arg(value).toUtf8();
    }
}

void SettingsXResource::processXsettingsChanged(const QString &key)
{
    switch (shash(key.toLatin1().data()))
    {
    case CONNECT(SETTINGS_SCHEMA_WINDOW_SCALING_FACTOR, _hash):
    case CONNECT(SETTINGS_SCHEMA_XFT_ANTIALIAS, _hash):
    case CONNECT(SETTINGS_SCHEMA_XFT_HINTING, _hash):
    case CONNECT(SETTINGS_SCHEMA_XFT_HINT_STYLE, _hash):
    case CONNECT(SETTINGS_SCHEMA_XFT_RGBA, _hash):
    case CONNECT(SETTINGS_SCHEMA_XFT_DPI, _hash):
    case CONNECT(SETTINGS_SCHEMA_GTK_CURSOR_THEME_NAME, _hash):
    case CONNECT(SETTINGS_SCHEMA_GTK_CURSOR_THEME_SIZE, _hash):
        updateProperties();
        break;
    default:
        break;
    }
}
}  // namespace Kiran