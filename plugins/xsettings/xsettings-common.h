/*
 * @Author       : tangjie02
 * @Date         : 2020-11-24 10:22:11
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-11-24 16:15:35
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/plugins/xsettings/xsettings-common.h
 */
#pragma once

#define XSETTINGS_SCHEMA_ID "com.kylinsec.kiran.xsettings"
#define XSETTINGS_SCHEMA_DOUBLE_CLICK_TIME "double-click-time"
#define XSETTINGS_SCHEMA_DOUBLE_CLICK_DISTANCE "double-click-distance"
#define XSETTINGS_SCHEMA_DND_DRAG_THRESHOLD "dnd-drag-threshold"
#define XSETTINGS_SCHEMA_CURSOR_BLINK "cursor-blink"
#define XSETTINGS_SCHEMA_CURSOR_BLINK_TIME "cursor-blink-time"
#define XSETTINGS_SCHEMA_THEME_NAME "theme-name"
#define XSETTINGS_SCHEMA_XFT_ANTIALIAS "xft-antialias"
#define XSETTINGS_SCHEMA_ICON_THEME_NAME "icon-theme-name"
#define XSETTINGS_SCHEMA_XFT_HINTING "xft-hinting"
#define XSETTINGS_SCHEMA_XFT_HINT_STYLE "xft-hint-style"
#define XSETTINGS_SCHEMA_XFT_RGBA "xft-rgba"
#define XSETTINGS_SCHEMA_XFT_DPI "xft-dpi"
#define XSETTINGS_SCHEMA_GTK_CURSOR_THEME_NAME "gtk-cursor-theme-name"
#define XSETTINGS_SCHEMA_GTK_CURSOR_THEME_SIZE "gtk-cursor-theme-size"
#define XSETTINGS_SCHEMA_GTK_CAN_CHANGE_ACCELS "gtk-can-change-accels"
#define XSETTINGS_SCHEMA_GTK_COLOR_PALETTE "gtk-color-palette"
#define XSETTINGS_SCHEMA_GTK_FONT_NAME "gtk-font-name"
#define XSETTINGS_SCHEMA_GTK_ICON_SIZES "gtk-icon-sizes"
#define XSETTINGS_SCHEMA_GTK_KEY_THEME_NAME "gtk-key-theme-name"
#define XSETTINGS_SCHEMA_GTK_TOOLBAR_STYLE "gtk-toolbar-style"
#define XSETTINGS_SCHEMA_GTK_TOOLBAR_ICONS_SIZE "gtk-toolbar-icons-size"
#define XSETTINGS_SCHEMA_GTK_IM_PREEDIT_STYLE "gtk-im-preedit-style"
#define XSETTINGS_SCHEMA_GTK_IM_STATUS_STYLE "gtk-im-status-style"
#define XSETTINGS_SCHEMA_GTK_MENU_IMAGES "gtk-menu-images"
#define XSETTINGS_SCHEMA_GTK_BUTTON_IMAGES "gtk-button-images"
#define XSETTINGS_SCHEMA_GTK_MENUBAR_ACCEL "gtk-menubar-accel"
#define XSETTINGS_SCHEMA_WINDOW_SCALING_FACTOR "window-scaling-factor"
#define XSETTINGS_SCHEMA_WINDOW_SCALING_FACTOR_QT_SYNC "window-scaling-factor-qt-sync"

/* X servers sometimes lie about the screen's physical dimensions, so we cannot
 * compute an accurate DPI value.  When this happens, the user gets fonts that
 * are too huge or too tiny.  So, we see what the server returns:  if it reports
 * something outside of the range [DPI_LOW_REASONABLE_VALUE,
 * DPI_HIGH_REASONABLE_VALUE], then we assume that it is lying and we use
 * DPI_FALLBACK instead.
 *
 * See get_dpi_from_gsettings_or_server() below, and also
 * https://bugzilla.novell.com/show_bug.cgi?id=217790
 */
#define DPI_FALLBACK 96
#define DPI_LOW_REASONABLE_VALUE 50
#define DPI_HIGH_REASONABLE_VALUE 500

/* The minimum resolution at which we turn on a window-scale of 2 */
#define HIDPI_LIMIT (DPI_FALLBACK * 2)

/* The minimum screen height at which we turn on a window-scale of 2;
 * below this there just isn't enough vertical real estate for GNOME
 * apps to work, and it's better to just be tiny */
#define HIDPI_MIN_HEIGHT 1500

#define WM_COMMON_MARCO "Metacity (Marco)"
#define WM_COMMON_SAWFISH "Sawfish"
#define WM_COMMON_METACITY "Metacity"
#define WM_COMMON_COMPIZ "Compiz"
#define WM_COMMON_COMPIZ_OLD "compiz"
#define WM_COMMON_UNKNOWN "Unknown"