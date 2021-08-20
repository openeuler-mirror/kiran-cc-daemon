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
