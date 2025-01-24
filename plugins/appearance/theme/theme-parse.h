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
#include <QSharedPointer>
#include "lib/base/base.h"
#include "theme-data.h"
#include "theme-monitor.h"

namespace Kiran
{

class ThemeParse : public QObject
{
    Q_OBJECT

public:
    ThemeParse(QSharedPointer<ThemeMonitorInfo> monitorInfo);
    virtual ~ThemeParse() {};

    QSharedPointer<ThemeBase> parse();
    QSharedPointer<ThemeBase> parseBase();

private:
    QSharedPointer<ThemeBase> parseMeta();
    QSharedPointer<ThemeBase> parseGtk();
    QSharedPointer<ThemeBase> parseMetacity();
    QSharedPointer<ThemeBase> parseIcon();
    QSharedPointer<ThemeBase> parseCursor();

    QSharedPointer<ThemeBase> fileBase(QSharedPointer<ThemeBase> themeBase, AppearanceThemeType type);
    QString getThemePath(const QString& path, AppearanceThemeType type);

private:
    QSharedPointer<ThemeMonitorInfo> m_monitorInfo;

    // <ThemeMonitorType, AppearanceThemeType>
    static QMap<int, int> m_monitor2Theme;
};
}  // namespace Kiran