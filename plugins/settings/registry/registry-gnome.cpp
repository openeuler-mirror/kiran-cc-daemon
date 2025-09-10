/**
 * Copyright (c) 2025 ~ 2026 KylinSec Co., Ltd.
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

#include "registry-gnome.h"
#include <QGSettings>
#include "lib/base/base.h"
#include "settings-i.h"

namespace Kiran
{
#define GNOME_DESKTOP_SCHEMA_ID "org.gnome.desktop.interface"
#define GNOME_MOUSE_SCHEMA_ID "org.gnome.settings-daemon.peripherals.mouse"
#define GNOME_SOUND_SCHEMA_ID "org.gnome.desktop.sound"
#define GNOME_XSETTINGS_SCHEMA_ID "org.gnome.settings-daemon.plugins.xsettings"

const QMap<QString, QString> RegistryGnome::s_schema2GnomeSchema = {
    {SETTINGS_SCHEMA_NET_THEME_NAME, "gtk-theme"},
    {SETTINGS_SCHEMA_NET_ICON_THEME_NAME, "icon-theme"},
    {SETTINGS_SCHEMA_GTK_CURSOR_THEME_NAME, "cursor-theme"},
    {SETTINGS_SCHEMA_GTK_CURSOR_THEME_SIZE, "cursor-size"},
    {SETTINGS_SCHEMA_GTK_FONT_NAME, "font-name"},
    {SETTINGS_SCHEMA_NET_CURSOR_BLINK, "cursor-blink"},
    {SETTINGS_SCHEMA_NET_CURSOR_BLINK_TIME, "cursor-blink-time"},
    {SETTINGS_SCHEMA_GTK_CURSOR_BLINK_TIMEOUT, "cursor-blink-timeout"},
    {SETTINGS_SCHEMA_GTK_IM_MODULE, "gtk-im-module"},
    {SETTINGS_SCHEMA_GTK_ENABLE_ANIMATIONS, "enable-animations"},
    {SETTINGS_SCHEMA_GTK_ENABLE_PRIMARY_PASTE, "gtk-enable-primary-paste"},
    {SETTINGS_SCHEMA_NET_DOUBLE_CLICK_TIME, "double-click"},
    {SETTINGS_SCHEMA_NET_DND_DRAG_THRESHOLD, "drag-threshold"},
    {SETTINGS_SCHEMA_NET_SOUND_THEME_NAME, "theme-name"},
    {SETTINGS_SCHEMA_NET_ENABLE_EVENT_SOUNDS, "event-sounds"},
    {SETTINGS_SCHEMA_NET_ENABLE_INPUT_FEEDBACK_SOUNDS, "input-feedback-sounds"},
    {SETTINGS_SCHEMA_XFT_ANTIALIAS, "antialiasing"},
    {SETTINGS_SCHEMA_XFT_HINT_STYLE, "hinting"},
    {SETTINGS_SCHEMA_XFT_RGBA, "rgba-order"}};

RegistryGnome::RegistryGnome(QObject *parent) : QObject(parent),
                                                m_settings(nullptr),
                                                m_gnomeDesktopSettings(nullptr),
                                                m_gnomeMouseSettings(nullptr),
                                                m_gnomeSoundSettings(nullptr),
                                                m_gnomeXSettingsSettings(nullptr)
{
    m_settings = new QGSettings(SETTINGS_SCHEMA_ID, "", this);

    if (QGSettings::isSchemaInstalled(GNOME_DESKTOP_SCHEMA_ID))
    {
        m_gnomeDesktopSettings = new QGSettings(GNOME_DESKTOP_SCHEMA_ID, "", this);
    }

    if (QGSettings::isSchemaInstalled(GNOME_MOUSE_SCHEMA_ID))
    {
        m_gnomeMouseSettings = new QGSettings(GNOME_MOUSE_SCHEMA_ID, "", this);
    }

    if (QGSettings::isSchemaInstalled(GNOME_SOUND_SCHEMA_ID))
    {
        m_gnomeSoundSettings = new QGSettings(GNOME_SOUND_SCHEMA_ID, "", this);
    }

    if (QGSettings::isSchemaInstalled(GNOME_XSETTINGS_SCHEMA_ID))
    {
        m_gnomeXSettingsSettings = new QGSettings(GNOME_XSETTINGS_SCHEMA_ID, "", this);
    }
}

void RegistryGnome::init()
{
    for (const auto &key : m_settings->keys())
    {
        sync2GnomeSettings(key);
    }

    connect(m_settings, &QGSettings::changed, std::bind(&RegistryGnome::sync2GnomeSettings, this, std::placeholders::_1));
}

QString RegistryGnome::xftAntialias2Gnome(int antialias)
{
    if (antialias == 0)
    {
        return "none";
    }
    return "grayscale";
}

QString RegistryGnome::xftHintStyle2Gnome(const QString &hintStyle)
{
    switch (shash(hintStyle.toLatin1().data()))
    {
    case CONNECT("hintnone", _hash):
        return "none";
    case CONNECT("hintslight", _hash):
        return "slight";
    case CONNECT("hintmedium", _hash):
        return "medium";
    case CONNECT("hintfull", _hash):
        return "full";
    default:
        break;
    }
    return "none";
}

void RegistryGnome::sync2GnomeSettings(const QString &key)
{
    auto value = m_settings->get(key);
    auto gtkKey = s_schema2GnomeSchema[key];
    QGSettings *gtkSettings = nullptr;

    switch (shash(key.toLatin1().data()))
    {
    case CONNECT(SETTINGS_SCHEMA_NET_THEME_NAME, _hash):
    case CONNECT(SETTINGS_SCHEMA_NET_ICON_THEME_NAME, _hash):
    case CONNECT(SETTINGS_SCHEMA_GTK_CURSOR_THEME_NAME, _hash):
    case CONNECT(SETTINGS_SCHEMA_GTK_CURSOR_THEME_SIZE, _hash):
    case CONNECT(SETTINGS_SCHEMA_GTK_FONT_NAME, _hash):
    case CONNECT(SETTINGS_SCHEMA_NET_CURSOR_BLINK, _hash):
    case CONNECT(SETTINGS_SCHEMA_NET_CURSOR_BLINK_TIME, _hash):
    case CONNECT(SETTINGS_SCHEMA_GTK_CURSOR_BLINK_TIMEOUT, _hash):
    case CONNECT(SETTINGS_SCHEMA_GTK_ENABLE_ANIMATIONS, _hash):
    case CONNECT(SETTINGS_SCHEMA_GTK_ENABLE_PRIMARY_PASTE, _hash):
    {
        gtkSettings = m_gnomeDesktopSettings;
        break;
    }
    case CONNECT(SETTINGS_SCHEMA_NET_DOUBLE_CLICK_TIME, _hash):
    case CONNECT(SETTINGS_SCHEMA_NET_DND_DRAG_THRESHOLD, _hash):
    {
        gtkSettings = m_gnomeMouseSettings;
        break;
    }
    case CONNECT(SETTINGS_SCHEMA_NET_SOUND_THEME_NAME, _hash):
    case CONNECT(SETTINGS_SCHEMA_NET_ENABLE_EVENT_SOUNDS, _hash):
    case CONNECT(SETTINGS_SCHEMA_NET_ENABLE_INPUT_FEEDBACK_SOUNDS, _hash):
    {
        gtkSettings = m_gnomeSoundSettings;
        break;
    }
    case CONNECT(SETTINGS_SCHEMA_XFT_ANTIALIAS, _hash):
    {
        gtkSettings = m_gnomeXSettingsSettings;
        value = xftAntialias2Gnome(value.toInt());
        break;
    }
    case CONNECT(SETTINGS_SCHEMA_XFT_HINT_STYLE, _hash):
    {
        gtkSettings = m_gnomeXSettingsSettings;
        value = xftHintStyle2Gnome(value.toString());
        break;
    }
    case CONNECT(SETTINGS_SCHEMA_XFT_RGBA, _hash):
    {
        gtkSettings = m_gnomeXSettingsSettings;
        break;
    }
    case CONNECT(SETTINGS_SCHEMA_GTK_IM_MODULE, _hash):  // gnome中的gtk-im-module有其他程序也在修改，这里先忽略
    case CONNECT(SETTINGS_SCHEMA_NET_DOUBLE_CLICK_DISTANCE, _hash):
    case CONNECT(SETTINGS_SCHEMA_XFT_DPI, _hash):
    case CONNECT(SETTINGS_SCHEMA_XFT_HINTING, _hash):
    case CONNECT(SETTINGS_SCHEMA_GTK_KEY_THEME_NAME, _hash):
    case CONNECT(SETTINGS_SCHEMA_GTK_TOOLBAR_STYLE, _hash):
    case CONNECT(SETTINGS_SCHEMA_GTK_TOOLBAR_ICONS_SIZE, _hash):
    case CONNECT(SETTINGS_SCHEMA_GTK_IM_PREEDIT_STYLE, _hash):
    case CONNECT(SETTINGS_SCHEMA_GTK_IM_STATUS_STYLE, _hash):
    case CONNECT(SETTINGS_SCHEMA_GTK_MENU_IMAGES, _hash):
    case CONNECT(SETTINGS_SCHEMA_GTK_BUTTON_IMAGES, _hash):
    case CONNECT(SETTINGS_SCHEMA_GTK_MENUBAR_ACCEL, _hash):
    case CONNECT(SETTINGS_SCHEMA_GTK_COLOR_SCHEME, _hash):
    case CONNECT(SETTINGS_SCHEMA_GTK_FILE_CHOOSER_BACKEND, _hash):
    case CONNECT(SETTINGS_SCHEMA_GTK_DECORATION_LAYOUT, _hash):
    case CONNECT(SETTINGS_SCHEMA_GTK_SHELL_SHOWS_APP_MENU, _hash):
    case CONNECT(SETTINGS_SCHEMA_GTK_SHELL_SHOWS_MENUBAR, _hash):
    case CONNECT(SETTINGS_SCHEMA_GTK_SHOW_INPUT_METHOD_MENU, _hash):
    case CONNECT(SETTINGS_SCHEMA_GTK_SHOW_UNICODE_MENU, _hash):
    case CONNECT(SETTINGS_SCHEMA_GTK_AUTOMATIC_MNEMONICS, _hash):
    case CONNECT(SETTINGS_SCHEMA_GTK_DIALOGS_USE_HEADER, _hash):
    case CONNECT(SETTINGS_SCHEMA_WINDOW_SCALING_FACTOR, _hash):
    case CONNECT(SETTINGS_SCHEMA_WINDOW_SCALING_FACTOR_QT_SYNC, _hash):
    case CONNECT(SETTINGS_SCHEMA_RELOAD_WHEN_SCALING, _hash):
    case CONNECT(SETTINGS_SCHEMA_FONT_DPI, _hash):
    {
        // Ignore these properties
        break;
    }

    default:
        KLOG_WARNING(settings) << "Unknown key" << key;
        break;
    }

    /* 下列属性未设置，还需要进一步分析必要性：
        static TranslationEntry translations[] = {
    {FALSE, "org.gnome.desktop.privacy", "recent-files-max-age", "gtk-recent-files-max-age", G_TYPE_INT, {.i = 30}},
    {FALSE, "org.gnome.desktop.privacy", "remember-recent-files", "gtk-recent-files-enabled", G_TYPE_BOOLEAN, {.b = TRUE}},
    {FALSE, WM_SETTINGS_SCHEMA, "button-layout", "gtk-decoration-layout", G_TYPE_STRING, {.s = "menu:close"}},
    {FALSE, "org.gnome.desktop.interface", "text-scaling-factor", "gtk-xft-dpi", G_TYPE_NONE, {.i = 0}},
    {FALSE, "org.gnome.desktop.wm.preferences", "action-double-click-titlebar", "gtk-titlebar-double-click", G_TYPE_STRING, {.s = "toggle-maximize"}},
    {FALSE, "org.gnome.desktop.wm.preferences", "action-middle-click-titlebar", "gtk-titlebar-middle-click", G_TYPE_STRING, {.s = "none"}},
    {FALSE, "org.gnome.desktop.wm.preferences", "action-right-click-titlebar", "gtk-titlebar-right-click", G_TYPE_STRING, {.s = "menu"}},
    {FALSE, "org.gnome.desktop.a11y", "always-show-text-caret", "gtk-keynav-use-caret", G_TYPE_BOOLEAN, {.b = FALSE}}}; */

    if (gtkSettings)
    {
        if (gtkKey.isEmpty())
        {
            KLOG_WARNING(settings) << "settings schema" << key << "not found in s_schema2GnomeSchema.";
            return;
        }

        if (gtkSettings->get(gtkKey) != value)
        {
            gtkSettings->set(gtkKey, value);
        }
    }
}

}  // namespace Kiran
