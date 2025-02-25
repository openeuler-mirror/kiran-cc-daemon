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

#include <QString>
#include "appearance-i.h"

namespace Kiran
{
struct ThemeBase;

// 主题类型和主题名
using ThemeKey = QPair<int32_t, QString>;
// 主题类型/主题名/优先级
using ThemeUniqueKey = std::tuple<int32_t, QString, int32_t>;
// 优先级'值'越小的排在前面
using ThemeKeyValue = QMap<int32_t, QSharedPointer<ThemeBase>>;

enum ThemeMonitorEventType
{
    // META主题目录添加/删除/变化
    TMET_META_ADD,
    TMET_META_DEL,
    TMET_META_CHG,
    // GTK主题目录添加/删除/变化
    TMET_GTK_ADD,
    TMET_GTK_DEL,
    TMET_GTK_CHG,
    // METACITY主题目录添加/删除/变化
    TMET_METACITY_ADD,
    TMET_METACITY_DEL,
    TMET_METACITY_CHG,
    // 图标主题目录添加/删除/变化
    TMET_ICON_ADD,
    TMET_ICON_DEL,
    TMET_ICON_CHG,
    // 光标主题目录添加/删除/变化
    TMET_CURSOR_ADD,
    TMET_CURSOR_DEL,
    TMET_CURSOR_CHG,
};

enum ThemeMonitorType
{
    // META主题父目录监控
    THEME_MONITOR_TYPE_META_PARENT,
    // META主题目录监控
    THEME_MONITOR_TYPE_META,
    // GTK主题目录监控
    THEME_MONITOR_TYPE_GTK,
    // 窗口主题目录监控
    THEME_MONITOR_TYPE_METACITY,
    // 图标主题父目录监控
    THEME_MONITOR_TYPE_ICON_PARENT,
    // 图标主题目录监控
    THEME_MONITOR_TYPE_ICON,
    // 光标主题目录监控
    THEME_MONITOR_TYPE_CURSOR,
};

struct ThemeBase
{
    // 主题类型
    AppearanceThemeType type;
    // 主题名
    QString name;
    // 主题加载优先级
    int32_t priority;
    // 主题路径
    QString path;
};

struct ThemeMeta : public ThemeBase
{
    QString gtk_theme;
    QString metacity_theme;
    QString icon_theme;
    QString cursor_theme;
};

using ThemeInfoVec = std::vector<QSharedPointer<ThemeBase>>;
}  // namespace Kiran