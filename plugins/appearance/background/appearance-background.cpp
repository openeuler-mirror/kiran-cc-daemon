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

#include "appearance-background.h"
#include <QApplication>
#include <QGSettings>
#include <QScreen>
#include "appearance-i.h"
#include "background-view.h"
#include "lib/base/base.h"

namespace Kiran
{
#define MATE_BACKGROUND_SCHEMA_ID "org.mate.background"
#define MATE_BACKGROUND_SCHAME_KEY_PICTURE_FILENAME "pictureFilename"

AppearanceBackground::AppearanceBackground(QObject *parent) : QObject(parent)
{
    m_mateBackgroundSettings = new QGSettings(MATE_BACKGROUND_SCHEMA_ID, "", this);
    m_appearanceSettings = new QGSettings(APPEARANCE_SCHAME_ID, "", this);
    m_desktopView = nullptr;
}

void AppearanceBackground::init()
{
    m_desktopBackground = m_appearanceSettings->get(APPEARANCE_SCHEMA_KEY_DESKTOP_BG).toString();
    if (m_appearanceSettings->get(APPEARANCE_SCHEMA_KEY_SHOW_DESKTOP_BACKGROUND).toBool())
    {
        m_desktopView = new DesktopView(this);
        if (!m_desktopBackground.isEmpty())
        {
            m_desktopView->setBackground(m_desktopBackground);
        }
    }

    connect(m_appearanceSettings, &QGSettings::changed, this, &AppearanceBackground::processAppearanceSettingsChanged);
}

void AppearanceBackground::setBackground(const QString &path)
{
    KLOG_INFO(appearance) << "setBackground" << path;

    RETURN_IF_TRUE(m_desktopBackground == path);
    m_desktopBackground = path;

    // 兼容老的mate桌面设置
    m_mateBackgroundSettings->set(MATE_BACKGROUND_SCHAME_KEY_PICTURE_FILENAME, path);

    if (m_desktopView)
    {
        m_desktopView->setBackground(m_desktopBackground);
    }
}

void AppearanceBackground::updateBackground(const bool &isShow)
{
    KLOG_INFO(appearance) << "updateBackground" << isShow;

    if (isShow)
    {
        if (m_desktopView)
        {
            return;
        }
        m_desktopView = new DesktopView(this);
        if (!m_desktopBackground.isEmpty())
        {
            m_desktopView->setBackground(m_desktopBackground);
        }
    }
    else
    {
        if (m_desktopView)
        {
            delete m_desktopView;
            m_desktopView = nullptr;
        }
    }
}

void AppearanceBackground::processAppearanceSettingsChanged(const QString &key)
{
    switch (shash(key.toLatin1().data()))
    {
    case CONNECT(APPEARANCE_SCHEMA_KEY_DESKTOP_BG, _hash):
        setBackground(m_appearanceSettings->get(APPEARANCE_SCHEMA_KEY_DESKTOP_BG).toString());
        break;
    case CONNECT(APPEARANCE_SCHEMA_KEY_SHOW_DESKTOP_BACKGROUND, _hash):
        updateBackground(m_appearanceSettings->get(APPEARANCE_SCHEMA_KEY_SHOW_DESKTOP_BACKGROUND).toBool());
        break;
    default:
        break;
    }
}

}  // namespace Kiran
