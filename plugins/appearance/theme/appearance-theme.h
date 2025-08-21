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

#pragma once

#include <QMap>
#include <QObject>
#include <QPair>
#include <QSharedPointer>
#include "error-i.h"
#include "theme-data.h"

class QGSettings;

namespace Kiran
{
class ThemeMonitor;
class ThemeMonitorInfo;

enum ThemeEventType
{
    // 主题添加
    THEME_EVENT_TYPE_ADD,
    // 主题删除
    THEME_EVENT_TYPE_DEL,
    // 主题修改
    THEME_EVENT_TYPE_CHG,
};

class AppearanceTheme : public QObject
{
    Q_OBJECT

public:
    AppearanceTheme(QObject* parent = nullptr);
    virtual ~AppearanceTheme() {};

    void init();

    // 获取指定类型的主题列表
    ThemeInfoVec getThemesByType(AppearanceThemeType type);
    QSharedPointer<ThemeBase> getTheme(ThemeUniqueKey unique_key);
    // 如果存在多个类型和名字相同的主题，则返回优先级最高的主题
    QSharedPointer<ThemeBase> getTheme(ThemeKey key);

    // 设置指定类型(meta/gtk/metacity...)的主题，如果主题不存在则返回失败，否则修改gsettings使主题生效
    bool setTheme(ThemeKey key, CCErrorCode& errorCode);
    // 获取指定类型的主题名，直接从gsettings中读取
    QString getTheme(AppearanceThemeType type);

Q_SIGNALS:
    // 某个类型的主题设置发生了变化
    void themeChanged(ThemeKey key);
    void themeDetailChanged(ThemeKey key, ThemeEventType type);

private:
    bool addTheme(QSharedPointer<ThemeBase> theme);
    bool replaceTheme(QSharedPointer<ThemeBase> theme);
    bool delTheme(ThemeUniqueKey unique_key);

    void setMetaTheme(QSharedPointer<ThemeMeta> theme);
    void setGtkTheme(const QString& themeName);
    void setIconTheme(const QString& themeName);
    void setCursorTheme(const QString& themeName);
    void setMetacityTheme(const QString& themeName);
    void trySyncGnomeColorSchema();
    QString themeEnum2Str(AppearanceThemeType type);

    // xsettings插件的settings项发生变化处理
    void processXSettingsSettingsChanged(const QString& key);
    void processMonitorInfoChanged(QSharedPointer<ThemeMonitorInfo> monitorInfo, ThemeMonitorEventType eventType);

private:
    ThemeMonitor* m_themeMonitor;

    QMap<ThemeKey, ThemeKeyValue> m_themes;

    QGSettings* m_xsettingsSettings;
    QGSettings* m_marcoSettings;
    QGSettings* m_mouseSettings;
    QGSettings* m_gnomeDesktopSettigns;
};
}  // namespace Kiran