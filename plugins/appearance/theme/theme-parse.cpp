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

#include "theme-parse.h"
#include <QFileInfo>
#include <QSettings>

namespace Kiran
{
#define META_GROUP_NAME "X-GNOME-Metatheme"

QMap<int, int> ThemeParse::m_monitor2Theme = {
    {ThemeMonitorType::THEME_MONITOR_TYPE_META, APPEARANCE_THEME_TYPE_META},
    {ThemeMonitorType::THEME_MONITOR_TYPE_GTK, APPEARANCE_THEME_TYPE_GTK},
    {ThemeMonitorType::THEME_MONITOR_TYPE_METACITY, APPEARANCE_THEME_TYPE_METACITY},
    {ThemeMonitorType::THEME_MONITOR_TYPE_ICON, APPEARANCE_THEME_TYPE_ICON},
    {ThemeMonitorType::THEME_MONITOR_TYPE_CURSOR, APPEARANCE_THEME_TYPE_CURSOR}};

ThemeParse::ThemeParse(QSharedPointer<ThemeMonitorInfo> monitorInfo) : m_monitorInfo(monitorInfo)
{
}

QSharedPointer<ThemeBase> ThemeParse::parse()
{
    switch (m_monitorInfo->getType())
    {
    case ThemeMonitorType::THEME_MONITOR_TYPE_META:
        return parseMeta();
    case ThemeMonitorType::THEME_MONITOR_TYPE_GTK:
        return parseGtk();
    case ThemeMonitorType::THEME_MONITOR_TYPE_METACITY:
        return parseMetacity();
    case ThemeMonitorType::THEME_MONITOR_TYPE_ICON:
        return parseIcon();
    case ThemeMonitorType::THEME_MONITOR_TYPE_CURSOR:
        return parseCursor();
        break;
    default:
        return nullptr;
    }
}

QSharedPointer<ThemeBase> ThemeParse::parseBase()
{
    auto iter = ThemeParse::m_monitor2Theme.find(m_monitorInfo->getType());
    RETURN_VAL_IF_TRUE(iter == ThemeParse::m_monitor2Theme.end(), nullptr);

    auto base = QSharedPointer<ThemeBase>::create();
    base->type = AppearanceThemeType(iter.value());
    base->priority = m_monitorInfo->getPriority();
    base->path = getThemePath(m_monitorInfo->getPath(), base->type);
    base->name = QFileInfo(base->path).baseName();
    return base;
}

QSharedPointer<ThemeBase> ThemeParse::parseMeta()
{
    auto indexFile = QString("%1/%2").arg(m_monitorInfo->getPath()).arg("index.theme");
    RETURN_VAL_IF_FALSE(QFileInfo(indexFile).isFile(), nullptr);

    auto meta = QSharedPointer<ThemeMeta>::create();
    fileBase(meta, AppearanceThemeType::APPEARANCE_THEME_TYPE_META);

    // Gtk/Metacity/Icon三个主题必须设置，否则解析失败；Cursor主题可选

    QSettings settings(indexFile, QSettings::IniFormat);

    settings.beginGroup(META_GROUP_NAME);
    meta->gtk_theme = settings.value("GtkTheme").toString();
    meta->metacity_theme = settings.value("MetacityTheme").toString();
    meta->icon_theme = settings.value("IconTheme").toString();
    meta->cursor_theme = settings.value("CursorTheme").toString();
    settings.endGroup();

    return meta;
}

QSharedPointer<ThemeBase> ThemeParse::parseGtk()
{
    auto cssFile = QString("%1/%2").arg(m_monitorInfo->getPath()).arg("gtk.css");
    QFileInfo fileInfo(cssFile);

    RETURN_VAL_IF_FALSE(fileInfo.exists() && fileInfo.isFile(), nullptr);

    auto base = QSharedPointer<ThemeBase>::create();
    return fileBase(base, AppearanceThemeType::APPEARANCE_THEME_TYPE_GTK);
}

QSharedPointer<ThemeBase> ThemeParse::parseMetacity()
{
    auto theme1File = QString("%1/%2").arg(m_monitorInfo->getPath()).arg("metacity-theme-1.xml");
    auto theme2File = QString("%1/%2").arg(m_monitorInfo->getPath()).arg("metacity-theme-2.xml");
    auto theme3File = QString("%1/%2").arg(m_monitorInfo->getPath()).arg("metacity-theme-3.xml");

    QFileInfo file1Info(theme1File);
    QFileInfo file2Info(theme2File);
    QFileInfo file3Info(theme3File);

    RETURN_VAL_IF_TRUE(!file1Info.isFile() && !file2Info.isFile() && !file3Info.isFile(), nullptr);

    auto base = QSharedPointer<ThemeBase>::create();
    return fileBase(base, AppearanceThemeType::APPEARANCE_THEME_TYPE_METACITY);
}

QSharedPointer<ThemeBase> ThemeParse::parseIcon()
{
    auto indexFile = QString("%1/%2").arg(m_monitorInfo->getPath()).arg("index.theme");
    QFileInfo fileInfo(indexFile);

    RETURN_VAL_IF_FALSE(fileInfo.isFile(), nullptr);

    auto base = QSharedPointer<ThemeBase>::create();
    return fileBase(base, AppearanceThemeType::APPEARANCE_THEME_TYPE_ICON);
}

QSharedPointer<ThemeBase> ThemeParse::parseCursor()
{
    auto leftPtrFile = QString("%1/%2").arg(m_monitorInfo->getPath()).arg("left_ptr");
    QFileInfo fileInfo(leftPtrFile);

    RETURN_VAL_IF_FALSE(fileInfo.isFile(), nullptr);

    auto base = QSharedPointer<ThemeBase>::create();
    return fileBase(base, AppearanceThemeType::APPEARANCE_THEME_TYPE_CURSOR);
}

QSharedPointer<ThemeBase> ThemeParse::fileBase(QSharedPointer<ThemeBase> themeBase, AppearanceThemeType type)
{
    themeBase->type = type;
    themeBase->priority = m_monitorInfo->getPriority();
    themeBase->path = getThemePath(m_monitorInfo->getPath(), themeBase->type);
    themeBase->name = QFileInfo(themeBase->path).baseName();
    return themeBase;
}

QString ThemeParse::getThemePath(const QString& monitorPath, AppearanceThemeType type)
{
    if (type == APPEARANCE_THEME_TYPE_META ||
        type == APPEARANCE_THEME_TYPE_ICON)
    {
        return monitorPath;
    }
    else
    {
        return QFileInfo(monitorPath).absolutePath();
    }
}
}  // namespace Kiran
