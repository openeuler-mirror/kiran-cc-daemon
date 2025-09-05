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

#include "appearance-font.h"
#include <QGSettings>
#include "settings-i.h"

namespace Kiran
{
#define MARCO_SCHEMA_ID "org.mate.Marco.general"
#define MARCO_SCHAME_KEY_TITLEBAR_FONT "titlebarFont"

#define CAJA_SCHEMA_ID "org.mate.caja.desktop"
#define CAJA_SCHEMA_KEY_FONT "font"

#define INTERFACE_SCHEMA_ID "org.mate.interface"
#define INTERFACE_KEY_DOCUMENT_FONT_NAME "documentFontName"
#define INTERFACE_KEY_MONOSPACE_FONT_NAME "monospaceFontName"

AppearanceFont::AppearanceFont(QObject* parent) : QObject(parent)
{
    m_xsettingsSettings = new QGSettings(SETTINGS_SCHEMA_ID, "", this);
    m_interfaceSettings = new QGSettings(INTERFACE_SCHEMA_ID, "", this);
    m_marcoSettings = new QGSettings(MARCO_SCHEMA_ID, "", this);

    if (QGSettings::isSchemaInstalled(CAJA_SCHEMA_ID))
    {
        m_cajaSettings = new QGSettings(CAJA_SCHEMA_ID, "", this);
    }
}

void AppearanceFont::init()
{
#define BIND_FONT_CHANGED_SIGNAL(settings)                                                 \
    if (settings)                                                                          \
    {                                                                                      \
        connect(settings, &QGSettings::changed, this, &AppearanceFont::notifyFontChanged); \
    }

    BIND_FONT_CHANGED_SIGNAL(m_xsettingsSettings);
    BIND_FONT_CHANGED_SIGNAL(m_interfaceSettings);
    BIND_FONT_CHANGED_SIGNAL(m_marcoSettings);
    BIND_FONT_CHANGED_SIGNAL(m_cajaSettings);
}

QString AppearanceFont::getFont(AppearanceFontType type)
{
    switch (type)
    {
    case APPEARANCE_FONT_TYPE_APPLICATION:
    {
        RETURN_VAL_IF_FALSE(m_xsettingsSettings, QString());
        return m_xsettingsSettings->get(SETTINGS_SCHEMA_GTK_FONT_NAME).toString();
    }
    case APPEARANCE_FONT_TYPE_DOCUMENT:
    {
        RETURN_VAL_IF_FALSE(m_interfaceSettings, QString());
        return m_interfaceSettings->get(INTERFACE_KEY_DOCUMENT_FONT_NAME).toString();
    }
    case APPEARANCE_FONT_TYPE_DESKTOP:
    {
        RETURN_VAL_IF_FALSE(m_cajaSettings, QString());
        return m_cajaSettings->get(CAJA_SCHEMA_KEY_FONT).toString();
    }
    case APPEARANCE_FONT_TYPE_WINDOW_TITLE:
    {
        RETURN_VAL_IF_FALSE(m_marcoSettings, QString());
        return m_marcoSettings->get(MARCO_SCHAME_KEY_TITLEBAR_FONT).toString();
    }
    case APPEARANCE_FONT_TYPE_MONOSPACE:
    {
        RETURN_VAL_IF_FALSE(m_interfaceSettings, QString());
        return m_interfaceSettings->get(INTERFACE_KEY_MONOSPACE_FONT_NAME).toString();
    }
    default:
        KLOG_WARNING(appearance) << "Unknown font type" << type;
        return QString();
    }
}

bool AppearanceFont::setFont(AppearanceFontType type, const QString& font)
{
    KLOG_INFO(appearance) << "Set" << fontEnum2Str(type) << "font to" << font;

    switch (type)
    {
    case APPEARANCE_FONT_TYPE_APPLICATION:
    {
        RETURN_VAL_IF_FALSE(m_xsettingsSettings, false);
        m_xsettingsSettings->set(SETTINGS_SCHEMA_GTK_FONT_NAME, font);
        break;
    }
    case APPEARANCE_FONT_TYPE_DOCUMENT:
    {
        RETURN_VAL_IF_FALSE(m_interfaceSettings, false);
        m_interfaceSettings->set(INTERFACE_KEY_DOCUMENT_FONT_NAME, font);
        break;
    }
    case APPEARANCE_FONT_TYPE_DESKTOP:
    {
        RETURN_VAL_IF_FALSE(m_cajaSettings, false);
        m_cajaSettings->set(CAJA_SCHEMA_KEY_FONT, font);
        break;
    }
    case APPEARANCE_FONT_TYPE_WINDOW_TITLE:
    {
        RETURN_VAL_IF_FALSE(m_marcoSettings, false);
        m_marcoSettings->set(MARCO_SCHAME_KEY_TITLEBAR_FONT, font);
        break;
    }
    case APPEARANCE_FONT_TYPE_MONOSPACE:
    {
        RETURN_VAL_IF_FALSE(m_interfaceSettings, false);
        m_interfaceSettings->set(INTERFACE_KEY_MONOSPACE_FONT_NAME, font);
        break;
    }
    default:
        KLOG_WARNING(appearance) << "Unknown font type" << type;
        return false;
    }
    return true;
}

bool AppearanceFont::resetFont(AppearanceFontType type)
{
    switch (type)
    {
    case APPEARANCE_FONT_TYPE_APPLICATION:
    {
        RETURN_VAL_IF_FALSE(m_xsettingsSettings, false);
        m_xsettingsSettings->reset(SETTINGS_SCHEMA_GTK_FONT_NAME);
        break;
    }
    case APPEARANCE_FONT_TYPE_DOCUMENT:
    {
        RETURN_VAL_IF_FALSE(m_interfaceSettings, false);
        m_interfaceSettings->reset(INTERFACE_KEY_DOCUMENT_FONT_NAME);
        break;
    }
    case APPEARANCE_FONT_TYPE_DESKTOP:
    {
        RETURN_VAL_IF_FALSE(m_cajaSettings, false);
        m_cajaSettings->reset(CAJA_SCHEMA_KEY_FONT);
        break;
    }
    case APPEARANCE_FONT_TYPE_WINDOW_TITLE:
    {
        RETURN_VAL_IF_FALSE(m_marcoSettings, false);
        m_marcoSettings->reset(MARCO_SCHAME_KEY_TITLEBAR_FONT);
        break;
    }
    case APPEARANCE_FONT_TYPE_MONOSPACE:
    {
        RETURN_VAL_IF_FALSE(m_interfaceSettings, false);
        m_interfaceSettings->reset(INTERFACE_KEY_MONOSPACE_FONT_NAME);
        break;
    }
    default:
        KLOG_WARNING(appearance) << "Unknown font type" << type;
        return false;
    }
    return true;
}

QString AppearanceFont::fontEnum2Str(AppearanceFontType type)
{
    switch (type)
    {
    case APPEARANCE_FONT_TYPE_APPLICATION:
        return "application";
    case APPEARANCE_FONT_TYPE_DOCUMENT:
        return "document";
    case APPEARANCE_FONT_TYPE_DESKTOP:
        return "desktop";
    case APPEARANCE_FONT_TYPE_WINDOW_TITLE:
        return "windowTitle";
    case APPEARANCE_FONT_TYPE_MONOSPACE:
        return "monospace";
    default:
        break;
    }
    return "unknown";
}

void AppearanceFont::notifyFontChanged(const QString& key)
{
    switch (shash(key.toStdString().c_str()))
    {
    case CONNECT(SETTINGS_SCHEMA_GTK_FONT_NAME, _hash):
    {
        Q_EMIT fontChanged(AppearanceFontType::APPEARANCE_FONT_TYPE_APPLICATION,
                           getFont(AppearanceFontType::APPEARANCE_FONT_TYPE_APPLICATION));
        break;
    }
    case CONNECT(INTERFACE_KEY_DOCUMENT_FONT_NAME, _hash):
    {
        Q_EMIT fontChanged(AppearanceFontType::APPEARANCE_FONT_TYPE_DOCUMENT,
                           getFont(AppearanceFontType::APPEARANCE_FONT_TYPE_DOCUMENT));
        break;
    }
    case CONNECT(CAJA_SCHEMA_KEY_FONT, _hash):
    {
        Q_EMIT fontChanged(AppearanceFontType::APPEARANCE_FONT_TYPE_DESKTOP,
                           getFont(AppearanceFontType::APPEARANCE_FONT_TYPE_DESKTOP));
        break;
    }
    case CONNECT(MARCO_SCHAME_KEY_TITLEBAR_FONT, _hash):
    {
        Q_EMIT fontChanged(AppearanceFontType::APPEARANCE_FONT_TYPE_WINDOW_TITLE,
                           getFont(AppearanceFontType::APPEARANCE_FONT_TYPE_WINDOW_TITLE));
        break;
    }
    case CONNECT(INTERFACE_KEY_MONOSPACE_FONT_NAME, _hash):
    {
        Q_EMIT fontChanged(AppearanceFontType::APPEARANCE_FONT_TYPE_MONOSPACE,
                           getFont(AppearanceFontType::APPEARANCE_FONT_TYPE_MONOSPACE));
        break;
    }
    default:
        break;
    }
}
}  // namespace Kiran
