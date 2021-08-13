/**
 * @Copyright (C) 2020 ~ 2021 KylinSec Co., Ltd. 
 *
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; If not, see <http: //www.gnu.org/licenses/>. 
 */

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#define APPEARANCE_DBUS_NAME "com.kylinsec.Kiran.SessionDaemon.Appearance"
#define APPEARANCE_OBJECT_PATH "/com/kylinsec/Kiran/SessionDaemon/Appearance"
#define APPEARANCE_DBUS_INTERFACE_NAME "com.kylinsec.Kiran.SessionDaemon.Appearance"

    enum AppearanceFontType
    {
        // 应用程序字体
        APPEARANCE_FONT_TYPE_APPLICATION = 0,
        // 文档字体
        APPEARANCE_FONT_TYPE_DOCUMENT,
        // 桌面字体
        APPEARANCE_FONT_TYPE_DESKTOP,
        // 窗口标题字体
        APPEARANCE_FONT_TYPE_WINDOW_TITLE,
        // 等宽字体，一般用于终端
        APPEARANCE_FONT_TYPE_MONOSPACE,
        APPEARANCE_FONT_TYPE_LAST
    };

    enum AppearanceThemeType
    {
        // META主题，META主题包括了GTK/图标/光标等主题的设置
        APPEARANCE_THEME_TYPE_META = 0,
        // GTK主题（依赖GTK的版本）
        APPEARANCE_THEME_TYPE_GTK,
        // metacity主题
        APPEARANCE_THEME_TYPE_METACITY,
        // 图标主题
        APPEARANCE_THEME_TYPE_ICON,
        // 光标主题
        APPEARANCE_THEME_TYPE_CURSOR,
        APPEARANCE_THEME_TYPE_LAST,
    };

#ifdef __cplusplus
}
#endif
