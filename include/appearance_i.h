/*
 * @Author       : tangjie02
 * @Date         : 2020-12-01 11:32:30
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-12-01 15:26:02
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/include/appearance_i.h
 */
#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#define APPEARANCE_DBUS_NAME "com.kylinsec.Kiran.SessionDaemon.Appearance"
#define APPEARANCE_OBJECT_PATH "/com/kylinsec/Kiran/SessionDaemon/Appearance"

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

#ifdef __cplusplus
}
#endif
