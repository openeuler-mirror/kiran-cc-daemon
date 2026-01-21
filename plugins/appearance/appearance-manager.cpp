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

#include "appearance-manager.h"
#include <QGSettings>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTime>
#include "appearanceadaptor.h"
#include "background/appearance-background.h"
#include "font/appearance-font.h"
#include "theme/appearance-theme.h"

namespace Kiran
{
AppearanceManager::AppearanceManager()

{
    m_appearanceSettings = new QGSettings(APPEARANCE_SCHAME_ID, "", this);
    m_appearanceAdaptor = new AppearanceAdaptor(this);

    m_appearanceTheme = new AppearanceTheme(this);
    m_appearanceFont = new AppearanceFont(this);
    m_appearanceBackground = new AppearanceBackground(this);
}

AppearanceManager::~AppearanceManager()
{
}

AppearanceManager* AppearanceManager::m_instance = nullptr;
void AppearanceManager::globalInit()
{
    m_instance = new AppearanceManager();
    m_instance->init();
}

bool AppearanceManager::getAutoSwitchWindowTheme() const
{
    return m_appearanceSettings->get(APPEARANCE_SCHEMA_KEY_AUTO_SWITCH_WINDOW_THEME).toBool();
}

#define SEND_PROPERTY_NOTIFY(property, propertyHump)                          \
    QVariantMap changedProperties;                                            \
    changedProperties.insert(QStringLiteral(#property), get##propertyHump()); \
                                                                              \
    QDBusMessage signalMessage = QDBusMessage::createSignal(                  \
        APPEARANCE_OBJECT_PATH,                                               \
        QStringLiteral("org.freedesktop.DBus.Properties"),                    \
        QStringLiteral("PropertiesChanged"));                                 \
                                                                              \
    signalMessage.setArguments({                                              \
        APPEARANCE_DBUS_INTERFACE_NAME,                                       \
        changedProperties,                                                    \
        QStringList(),                                                        \
    });                                                                       \
    QDBusConnection::sessionBus().send(signalMessage);

void AppearanceManager::EnableAutoSwitchWindowTheme()
{
    auto currentValue = m_appearanceSettings->get(APPEARANCE_SCHEMA_KEY_AUTO_SWITCH_WINDOW_THEME).toBool();
    RETURN_IF_TRUE(currentValue);
    m_appearanceSettings->set(APPEARANCE_SCHEMA_KEY_AUTO_SWITCH_WINDOW_THEME, true);
    KLOG_INFO(appearance) << "Enable auto switch window theme.";
}

QString AppearanceManager::GetFont(int type)
{
    if (type < 0 || type >= int32_t(AppearanceFontType::APPEARANCE_FONT_TYPE_LAST))
    {
        DBUS_ERROR_REPLY_AND_RETVAL(QString(), CCErrorCode::ERROR_APPEARANCE_FONT_TYPE_INVALID_1);
    }
    return m_appearanceFont->getFont(AppearanceFontType(type));
}

QString AppearanceManager::GetTheme(int type)
{
    return m_appearanceTheme->getTheme(AppearanceThemeType(type));
}

QString AppearanceManager::GetThemes(int type)
{
    if (type < 0 || type >= int32_t(AppearanceThemeType::APPEARANCE_THEME_TYPE_LAST))
    {
        DBUS_ERROR_REPLY_AND_RETVAL(QString(), CCErrorCode::ERROR_APPEARANCE_THEME_TYPE_INVALID);
    }

    QJsonArray jsonThemes;

    auto themes = m_appearanceTheme->getThemesByType(AppearanceThemeType(type));
    for (uint32_t i = 0; i < themes.size(); ++i)
    {
        QJsonObject jsonTheme;
        jsonTheme["name"] = themes[i]->name;
        jsonTheme["path"] = themes[i]->path;
        jsonThemes.append(jsonTheme);
    }
    return QJsonDocument(jsonThemes).toJson(QJsonDocument::Compact);
}

int AppearanceManager::GetCursorSize()
{
    return m_appearanceTheme->getCursorSize();
}

void AppearanceManager::ResetFont(int type)
{
    if (type < 0 || type >= int32_t(AppearanceFontType::APPEARANCE_FONT_TYPE_LAST))
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_APPEARANCE_FONT_TYPE_INVALID_3);
    }

    if (!m_appearanceFont->resetFont(AppearanceFontType(type)))
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_APPEARANCE_RESET_FONT_FAILED);
    }
}

void AppearanceManager::SetDesktopBackground(const QString& desktopBackground)
{
    RETURN_IF_TRUE(m_desktopBackground == desktopBackground);
    m_desktopBackground = desktopBackground;

    if (desktopBackground != m_appearanceSettings->get(APPEARANCE_SCHEMA_KEY_DESKTOP_BG).toString())
    {
        m_appearanceSettings->set(APPEARANCE_SCHEMA_KEY_DESKTOP_BG, desktopBackground);
        KLOG_INFO(appearance) << "Set desktop background" << desktopBackground << "to settings.";
    }
    SEND_PROPERTY_NOTIFY(desktop_background, DesktopBackground)
}

void AppearanceManager::SetFont(int type, const QString& font)
{
    if (type < 0 || type >= int32_t(AppearanceFontType::APPEARANCE_FONT_TYPE_LAST))
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_APPEARANCE_FONT_TYPE_INVALID_2);
    }

    if (!m_appearanceFont->setFont(AppearanceFontType(type), font))
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_APPEARANCE_FONT_TYPE_UNSUPPORTED);
    }
}

void AppearanceManager::SetLockScreenBackground(const QString& lockScreenBackground)
{
    RETURN_IF_TRUE(m_lockScreenBackground == lockScreenBackground);
    m_lockScreenBackground = lockScreenBackground;

    if (lockScreenBackground != m_appearanceSettings->get(APPEARANCE_SCHEMA_KEY_LOCKSCREEN_BG).toString())
    {
        m_appearanceSettings->set(APPEARANCE_SCHEMA_KEY_LOCKSCREEN_BG, lockScreenBackground);
        KLOG_INFO(appearance) << "Set lock screen background" << lockScreenBackground << "to settings.";
    }
    SEND_PROPERTY_NOTIFY(lock_screen_background, LockScreenBackground)
}

void AppearanceManager::SetTheme(int type, const QString& themeName)
{
    ThemeKey key = qMakePair(type, themeName);
    CCErrorCode errorCode = CCErrorCode::SUCCESS;
    if (!m_appearanceTheme->setTheme(key, errorCode))
    {
        DBUS_ERROR_REPLY_AND_RET(errorCode);
    }

    // 如果手动设置了GTK或者窗口标题主题，则取消主题自动切换
    if (type == AppearanceThemeType::APPEARANCE_THEME_TYPE_GTK ||
        type == AppearanceThemeType::APPEARANCE_THEME_TYPE_METACITY ||
        type == AppearanceThemeType::APPEARANCE_THEME_TYPE_META )
    {
        m_appearanceSettings->set(APPEARANCE_SCHEMA_KEY_AUTO_SWITCH_WINDOW_THEME, false);
        KLOG_INFO(appearance) << "Because of manual setting theme, disable auto switch window theme.";
    }
}

void AppearanceManager::SetCursorSize(int size)
{
    if (size < 24 || size > 64)
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_APPEARANCE_CURSOR_SIZE_INVALID);
    }

    m_appearanceTheme->setCursorSize(size);
}

void AppearanceManager::init()
{
    QSignalBlocker blocker(this);

    m_appearanceTheme->init();
    m_appearanceFont->init();
    m_appearanceBackground->init();

    loadFromSettings();

    if (getAutoSwitchWindowTheme())
    {
        autoSwitchForWindowTheme();
    }

    connect(m_appearanceTheme, &AppearanceTheme::themeChanged, this, &AppearanceManager::NotifyThemeChanged);
    connect(m_appearanceFont, &AppearanceFont::fontChanged, this, &AppearanceManager::NotifyFontChanged);
    connect(m_appearanceSettings, &QGSettings::changed, this, &AppearanceManager::processSettingsChanged);

    auto sessionConnection = QDBusConnection::sessionBus();
    if (!sessionConnection.registerService(APPEARANCE_DBUS_NAME))
    {
        KLOG_WARNING(appearance) << "Failed to register dbus name:" << APPEARANCE_DBUS_NAME;
        return;
    }

    if (!sessionConnection.registerObject(APPEARANCE_OBJECT_PATH, APPEARANCE_DBUS_INTERFACE_NAME, this))
    {
        KLOG_ERROR(appearance) << "Can't register object:" << sessionConnection.lastError();
        return;
    }
}

void AppearanceManager::loadFromSettings()
{
    m_desktopBackground = m_appearanceSettings->get(APPEARANCE_SCHEMA_KEY_DESKTOP_BG).toString();
    m_lockScreenBackground = m_appearanceSettings->get(APPEARANCE_SCHEMA_KEY_LOCKSCREEN_BG).toString();
}

void AppearanceManager::autoSwitchForWindowTheme()
{
    auto currentHour = QTime::currentTime().hour();
    auto errorCode = CCErrorCode::SUCCESS;

    // 下午8点之后到早上8点之前判定为晚上，使用深色主题，否则使用浅色主题
    auto theme_name = (currentHour < 8 || currentHour > 20) ? APPEARANCE_DEFAULT_DARK_META_THEME : APPEARANCE_DEFAULT_LIGHT_META_THEME;
    if (!m_appearanceTheme->setTheme(qMakePair(AppearanceThemeType::APPEARANCE_THEME_TYPE_META, theme_name),
                                     errorCode))
    {
        KLOG_WARNING(appearance) << "Failed to set window meta theme. errorCode:" << errorCode;
    }
}

void AppearanceManager::NotifyThemeChanged(ThemeKey themeKey)
{
    Q_EMIT ThemeChanged(themeKey.first, themeKey.second);
}

void AppearanceManager::NotifyFontChanged(AppearanceFontType type, const QString& font)
{
    Q_EMIT FontChanged(type, font);
}

void AppearanceManager::processSettingsChanged(const QString& key)
{
    switch (shash(key.toLatin1().data()))
    {
    case CONNECT(APPEARANCE_SCHEMA_KEY_DESKTOP_BG, _hash):
    {
        SetDesktopBackground(m_appearanceSettings->get(key).toString());
        break;
    }
    case CONNECT(APPEARANCE_SCHEMA_KEY_LOCKSCREEN_BG, _hash):
    {
        SetLockScreenBackground(m_appearanceSettings->get(key).toString());
        break;
    }
    case CONNECT(APPEARANCE_SCHEMA_KEY_AUTO_SWITCH_WINDOW_THEME, _hash):
    {
        if (getAutoSwitchWindowTheme())
        {
            autoSwitchForWindowTheme();
        }
        break;
    }
    default:
        break;
    }
}

}  // namespace  Kiran
