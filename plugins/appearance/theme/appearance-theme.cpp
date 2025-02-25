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

#include "appearance-theme.h"
#include <QGSettings>
#include "lib/base/base.h"
#include "theme-monitor.h"
#include "theme-parse.h"
#include "xsettings-i.h"

namespace Kiran
{
#define MARCO_SCHEMA_ID "org.mate.Marco.general"
#define MARCO_SCHEMA_KEY_THEME "theme"

#define MOUSE_SCHEMA_ID "org.mate.peripherals-mouse"
#define MOUSE_SCHEMA_KEY_CURSOR_THEME "cursorTheme"
#define MOUSE_SCHEMA_KEY_CURSOR_SIZE "cursorSize"

#define GNOME_DESKTOP_SCHEMA_ID "org.gnome.desktop.interface"
#define GNOME_DESKTOP_SCHEMA_KEY_COLOR_SCHEMA "color-scheme"

enum GnomeDesktopColorScheme
{
    GNOME_COLOR_SCHEME_DEFAULT,
    GNOME_COLOR_SCHEME_PREFER_DARK,
    GNOME_COLOR_SCHEME_PREFER_LIGHT,
    GNOME_COLOR_SCHEME_LAST
};

AppearanceTheme::AppearanceTheme(QObject* parent) : QObject(parent),
                                                    m_themeMonitor(nullptr),
                                                    m_xsettingsSettings(nullptr),
                                                    m_marcoSettings(nullptr),
                                                    m_mouseSettings(nullptr),
                                                    m_gnomeDesktopSettigns(nullptr)
{
    m_themeMonitor = new ThemeMonitor(this);

    m_xsettingsSettings = new QGSettings(XSETTINGS_SCHEMA_ID, "", this);

    if (QGSettings::isSchemaInstalled(MARCO_SCHEMA_ID))
    {
        m_marcoSettings = new QGSettings(MARCO_SCHEMA_ID, "", this);
    }

    if (QGSettings::isSchemaInstalled(MOUSE_SCHEMA_ID))
    {
        m_mouseSettings = new QGSettings(MOUSE_SCHEMA_ID, "", this);
    }

    if (QGSettings::isSchemaInstalled(GNOME_DESKTOP_SCHEMA_ID))
    {
        m_gnomeDesktopSettigns = new QGSettings(GNOME_DESKTOP_SCHEMA_ID, "", this);
    }
}

void AppearanceTheme::init()
{
    m_themeMonitor->init();

    for (auto& monitorInfo : m_themeMonitor->getMonitorInfos())
    {
        ThemeParse parse(monitorInfo);
        auto theme = parse.parse();
        if (theme)
        {
            addTheme(theme);
        }
    }

    trySyncGnomeColorSchema();

    connect(m_xsettingsSettings, &QGSettings::changed, this, &AppearanceTheme::processXSettingsSettingsChanged);
    connect(m_themeMonitor, &ThemeMonitor::themeChanged, this, &AppearanceTheme::processMonitorInfoChanged);
}

ThemeInfoVec AppearanceTheme::getThemesByType(AppearanceThemeType type)
{
    ThemeInfoVec themes;
    for (auto key : m_themes.keys())
    {
        // QMap结构是按照优先级排序的，所以如果存在相同类型的主题，只取优先级最高的主题
        if (key.first == type && m_themes[key].size() > 0)
        {
            themes.push_back(m_themes[key].begin().value());
        }
    }
    return themes;
}

QSharedPointer<ThemeBase> AppearanceTheme::getTheme(ThemeUniqueKey unique_key)
{
    ThemeKey key = qMakePair(std::get<0>(unique_key), std::get<1>(unique_key));
    auto themesIter = m_themes.find(key);
    RETURN_VAL_IF_TRUE(themesIter == m_themes.end(), nullptr);

    auto themeIter = themesIter.value().find(std::get<2>(unique_key));
    RETURN_VAL_IF_TRUE(themeIter == themesIter.value().end(), nullptr);
    return themeIter.value();
}

QSharedPointer<ThemeBase> AppearanceTheme::getTheme(ThemeKey key)
{
    auto value = m_themes.value(key);
    RETURN_VAL_IF_TRUE(value.size() == 0, nullptr);
    return value.begin().value();
}

bool AppearanceTheme::setTheme(ThemeKey key, CCErrorCode& errorCode)
{
    auto theme = getTheme(key);
    if (!theme)
    {
        errorCode = CCErrorCode::ERROR_APPEARANCE_THEME_NOT_EXIST;
        return false;
    }

    KLOG_INFO(appearance) << "Set" << themeEnum2Str(theme->type) << "theme to " << theme->name;

    switch (theme->type)
    {
    case AppearanceThemeType::APPEARANCE_THEME_TYPE_META:
        setMetaTheme(theme.staticCast<ThemeMeta>());
        break;
    case AppearanceThemeType::APPEARANCE_THEME_TYPE_GTK:
        setGtkTheme(theme->name);
        trySyncGnomeColorSchema();
        break;
    case AppearanceThemeType::APPEARANCE_THEME_TYPE_METACITY:
        setMetacityTheme(theme->name);
        break;
    case AppearanceThemeType::APPEARANCE_THEME_TYPE_ICON:
        setIconTheme(theme->name);
        break;
    case AppearanceThemeType::APPEARANCE_THEME_TYPE_CURSOR:
        setCursorTheme(theme->name);
        break;
    default:
        errorCode = CCErrorCode::ERROR_APPEARANCE_THEME_TYPE_UNSUPPORTED;
        return false;
    }
    return true;
}

QString AppearanceTheme::getTheme(AppearanceThemeType type)
{
    switch (type)
    {
    case AppearanceThemeType::APPEARANCE_THEME_TYPE_GTK:
        return m_xsettingsSettings->get(XSETTINGS_SCHEMA_NET_THEME_NAME).toString();
    case AppearanceThemeType::APPEARANCE_THEME_TYPE_METACITY:
    {
        if (m_marcoSettings)
        {
            return m_marcoSettings->get(MARCO_SCHEMA_KEY_THEME).toString();
        }
        break;
    }
    case AppearanceThemeType::APPEARANCE_THEME_TYPE_ICON:
        return m_xsettingsSettings->get(XSETTINGS_SCHEMA_NET_ICON_THEME_NAME).toString();
    case AppearanceThemeType::APPEARANCE_THEME_TYPE_CURSOR:
        return m_xsettingsSettings->get(XSETTINGS_SCHEMA_GTK_CURSOR_THEME_NAME).toString();
    default:
        break;
    }
    return QString();
}

bool AppearanceTheme::addTheme(QSharedPointer<ThemeBase> theme)
{
    RETURN_VAL_IF_FALSE(theme, false);
    ThemeKey key = qMakePair(theme->type, theme->name);

    RETURN_VAL_IF_TRUE(m_themes[key].contains(theme->priority), false);
    m_themes[key][theme->priority] = theme;
    return true;
}

bool AppearanceTheme::replaceTheme(QSharedPointer<ThemeBase> theme)
{
    RETURN_VAL_IF_FALSE(theme, false);
    ThemeKey key = qMakePair(theme->type, theme->name);
    m_themes[key][theme->priority] = theme;
    return true;
}

bool AppearanceTheme::delTheme(ThemeUniqueKey uniqueKey)
{
    ThemeKey key = qMakePair(std::get<0>(uniqueKey), std::get<1>(uniqueKey));
    auto themesIter = m_themes.find(key);
    if (themesIter != m_themes.end())
    {
        auto themeIter = themesIter.value().find(std::get<2>(uniqueKey));
        if (themeIter != themesIter.value().end())
        {
            themesIter.value().erase(themeIter);
            return true;
        }
    }
    return false;
}

void AppearanceTheme::setMetaTheme(QSharedPointer<ThemeMeta> theme)
{
    setGtkTheme(theme->gtk_theme);
    setIconTheme(theme->icon_theme);
    setCursorTheme(theme->cursor_theme);
    setMetacityTheme(theme->metacity_theme);

    Q_EMIT themeChanged(qMakePair(AppearanceThemeType::APPEARANCE_THEME_TYPE_META, theme->name));
}

void AppearanceTheme::setGtkTheme(const QString& themeName)
{
    RETURN_IF_TRUE(themeName.isEmpty());
    m_xsettingsSettings->set(XSETTINGS_SCHEMA_NET_THEME_NAME, themeName);
    Q_EMIT themeChanged(qMakePair(AppearanceThemeType::APPEARANCE_THEME_TYPE_GTK, themeName));
}

void AppearanceTheme::setIconTheme(const QString& themeName)
{
    RETURN_IF_TRUE(themeName.isEmpty());
    m_xsettingsSettings->set(XSETTINGS_SCHEMA_NET_ICON_THEME_NAME, themeName);
    Q_EMIT themeChanged(qMakePair(AppearanceThemeType::APPEARANCE_THEME_TYPE_ICON, themeName));
}

void AppearanceTheme::setCursorTheme(const QString& themeName)
{
    RETURN_IF_TRUE(themeName.isEmpty());
    m_xsettingsSettings->set(XSETTINGS_SCHEMA_GTK_CURSOR_THEME_NAME, themeName);
    // 由于Marco是从org.mate.peripherals-mouse读取光标主题的，所以这里要做一下适配
    if (m_mouseSettings)
    {
        m_mouseSettings->set(MOUSE_SCHEMA_KEY_CURSOR_THEME, themeName);
    }
    Q_EMIT themeChanged(qMakePair(AppearanceThemeType::APPEARANCE_THEME_TYPE_CURSOR, themeName));
}

void AppearanceTheme::setMetacityTheme(const QString& themeName)
{
    RETURN_IF_TRUE(themeName.isEmpty());

    if (m_marcoSettings)
    {
        m_marcoSettings->set(MARCO_SCHEMA_KEY_THEME, themeName);
        Q_EMIT themeChanged(ThemeKey{AppearanceThemeType::APPEARANCE_THEME_TYPE_METACITY, themeName});
    }
}

void AppearanceTheme::trySyncGnomeColorSchema()
{
    if (!m_gnomeDesktopSettigns)
    {
        return;
    }

    if (!m_gnomeDesktopSettigns->keys().contains(GNOME_DESKTOP_SCHEMA_KEY_COLOR_SCHEMA))
    {
        return;
    }

    auto themeName = getTheme(APPEARANCE_THEME_TYPE_GTK);
    if (themeName == APPEARANCE_DEFAULT_DARK_GTK_THEME)
    {
        m_gnomeDesktopSettigns->set(GNOME_DESKTOP_SCHEMA_KEY_COLOR_SCHEMA, GNOME_COLOR_SCHEME_PREFER_DARK);
    }
    else
    {
        m_gnomeDesktopSettigns->set(GNOME_DESKTOP_SCHEMA_KEY_COLOR_SCHEMA, GNOME_COLOR_SCHEME_PREFER_LIGHT);
    }
}

QString AppearanceTheme::themeEnum2Str(AppearanceThemeType type)
{
    switch (type)
    {
    case AppearanceThemeType::APPEARANCE_THEME_TYPE_META:
        return "meta";
    case AppearanceThemeType::APPEARANCE_THEME_TYPE_GTK:
        return "gtk";
    case AppearanceThemeType::APPEARANCE_THEME_TYPE_METACITY:
        return "metacity";
    case AppearanceThemeType::APPEARANCE_THEME_TYPE_ICON:
        return "icon";
    case AppearanceThemeType::APPEARANCE_THEME_TYPE_CURSOR:
        return "cursor";
    default:
        break;
    }
    return "unknown";
}

void AppearanceTheme::processXSettingsSettingsChanged(const QString& key)
{
    switch (shash(key.toLatin1().data()))
    {
    case CONNECT(XSETTINGS_SCHEMA_NET_THEME_NAME, _hash):
        Q_EMIT themeChanged(QPair<AppearanceThemeType, QString>(AppearanceThemeType::APPEARANCE_THEME_TYPE_GTK,
                                                                getTheme(AppearanceThemeType::APPEARANCE_THEME_TYPE_GTK)));
        break;
    case CONNECT(XSETTINGS_SCHEMA_NET_ICON_THEME_NAME, _hash):
        Q_EMIT themeChanged(qMakePair(AppearanceThemeType::APPEARANCE_THEME_TYPE_ICON,
                                      getTheme(AppearanceThemeType::APPEARANCE_THEME_TYPE_ICON)));
        break;
    case CONNECT(XSETTINGS_SCHEMA_GTK_CURSOR_THEME_NAME, _hash):
        Q_EMIT themeChanged(qMakePair(AppearanceThemeType::APPEARANCE_THEME_TYPE_CURSOR,
                                      getTheme(AppearanceThemeType::APPEARANCE_THEME_TYPE_CURSOR)));
        break;
    default:
        break;
    }
}

void AppearanceTheme::processMonitorInfoChanged(QSharedPointer<ThemeMonitorInfo> monitorInfo,
                                                ThemeMonitorEventType eventType)
{
    ThemeParse parse(monitorInfo);
    auto base = parse.parseBase();
    RETURN_IF_FALSE(base);

    auto key = qMakePair(base->type, base->name);
    auto old_theme = getTheme(key);
    auto parsed_theme = parse.parse();

    if (parsed_theme)
    {
        replaceTheme(parsed_theme);
    }
    else
    {
        ThemeUniqueKey unique_key = std::make_tuple(base->type, base->name, monitorInfo->getPriority());
        delTheme(unique_key);
    }

    // 需要在执行替换或删除操作后再进行获取，因为不确定变动的是否为最高优先级的主题
    auto new_theme = getTheme(key);

    if (old_theme && !new_theme)
    {
        Q_EMIT themeDetailChanged(key, ThemeEventType::THEME_EVENT_TYPE_DEL);
    }
    else if (!old_theme && new_theme)
    {
        Q_EMIT themeDetailChanged(key, ThemeEventType::THEME_EVENT_TYPE_ADD);
    }
    else if (old_theme && new_theme)
    {
        Q_EMIT themeDetailChanged(key, ThemeEventType::THEME_EVENT_TYPE_CHG);
    }
}

}  // namespace Kiran