/**
 * @file          /kiran-cc-daemon/include/xsettings_i.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#define XSETTINGS_DBUS_NAME "com.kylinsec.Kiran.SessionDaemon.XSettings"
#define XSETTINGS_OBJECT_PATH "/com/kylinsec/Kiran/SessionDaemon/XSettings"
#define XSETTINGS_DBUS_INTERFACE_NAME "com.kylinsec.Kiran.SessionDaemon.XSettings"

#define XSETTINGS_SCHEMA_ID "com.kylinsec.kiran.xsettings"

#define XSETTINGS_SCHEMA_NET_DOUBLE_CLICK_TIME "net-double-click-time"
#define XSETTINGS_SCHEMA_NET_DOUBLE_CLICK_DISTANCE "net-double-click-distance"
#define XSETTINGS_SCHEMA_NET_DND_DRAG_THRESHOLD "net-dnd-drag-threshold"
#define XSETTINGS_SCHEMA_NET_CURSOR_BLINK "net-cursor-blink"
#define XSETTINGS_SCHEMA_NET_CURSOR_BLINK_TIME "net-cursor-blink-time"
#define XSETTINGS_SCHEMA_NET_THEME_NAME "net-theme-name"
#define XSETTINGS_SCHEMA_NET_ICON_THEME_NAME "net-icon-theme-name"
#define XSETTINGS_SCHEMA_NET_ENABLE_EVENT_SOUNDS "net-enable-event-sounds"
#define XSETTINGS_SCHEMA_NET_SOUND_THEME_NAME "net-sound-theme-name"
#define XSETTINGS_SCHEMA_NET_ENABLE_INPUT_FEEDBACK_SOUNDS "net-enable-input-feedback-sounds"

#define XSETTINGS_SCHEMA_XFT_ANTIALIAS "xft-antialias"
#define XSETTINGS_SCHEMA_XFT_HINTING "xft-hinting"
#define XSETTINGS_SCHEMA_XFT_HINT_STYLE "xft-hint-style"
#define XSETTINGS_SCHEMA_XFT_RGBA "xft-rgba"
#define XSETTINGS_SCHEMA_XFT_DPI "xft-dpi"

#define XSETTINGS_SCHEMA_GTK_CURSOR_THEME_NAME "gtk-cursor-theme-name"
#define XSETTINGS_SCHEMA_GTK_CURSOR_THEME_SIZE "gtk-cursor-theme-size"
#define XSETTINGS_SCHEMA_GTK_FONT_NAME "gtk-font-name"
#define XSETTINGS_SCHEMA_GTK_KEY_THEME_NAME "gtk-key-theme-name"
#define XSETTINGS_SCHEMA_GTK_TOOLBAR_STYLE "gtk-toolbar-style"
#define XSETTINGS_SCHEMA_GTK_TOOLBAR_ICONS_SIZE "gtk-toolbar-icons-size"
#define XSETTINGS_SCHEMA_GTK_IM_PREEDIT_STYLE "gtk-im-preedit-style"
#define XSETTINGS_SCHEMA_GTK_IM_STATUS_STYLE "gtk-im-status-style"
#define XSETTINGS_SCHEMA_GTK_IM_MODULE "gtk-im-module"
#define XSETTINGS_SCHEMA_GTK_MENU_IMAGES "gtk-menu-images"
#define XSETTINGS_SCHEMA_GTK_BUTTON_IMAGES "gtk-button-images"
#define XSETTINGS_SCHEMA_GTK_MENUBAR_ACCEL "gtk-menubar-accel"
#define XSETTINGS_SCHEMA_GTK_COLOR_SCHEME "gtk-color-scheme"
#define XSETTINGS_SCHEMA_GTK_FILE_CHOOSER_BACKEND "gtk-file-chooser-backend"
#define XSETTINGS_SCHEMA_GTK_DECORATION_LAYOUT "gtk-decoration-layout"
#define XSETTINGS_SCHEMA_GTK_SHELL_SHOWS_APP_MENU "gtk-shell-shows-app-menu"
#define XSETTINGS_SCHEMA_GTK_SHELL_SHOWS_MENUBAR "gtk-shell-shows-menubar"
#define XSETTINGS_SCHEMA_GTK_SHOW_INPUT_METHOD_MENU "gtk-show-input-method-menu"
#define XSETTINGS_SCHEMA_GTK_SHOW_UNICODE_MENU "gtk-show-unicode-menu"
#define XSETTINGS_SCHEMA_GTK_AUTOMATIC_MNEMONICS "gtk-automatic-mnemonics"
#define XSETTINGS_SCHEMA_GTK_ENABLE_PRIMARY_PASTE "gtk-enable-primary-paste"
#define XSETTINGS_SCHEMA_GTK_ENABLE_ANIMATIONS "gtk-enable-animations"
#define XSETTINGS_SCHEMA_GTK_DIALOGS_USE_HEADER "gtk-dialogs-use-header"
#define XSETTINGS_SCHEMA_WINDOW_SCALING_FACTOR "window-scaling-factor"
#define XSETTINGS_SCHEMA_WINDOW_SCALING_FACTOR_QT_SYNC "window-scaling-factor-qt-sync"

#define XSETTINGS_REGISTRY_PROP_NET_DOUBLE_CLICK_TIME "Net/DoubleClickTime"
#define XSETTINGS_REGISTRY_PROP_NET_DOUBLE_CLICK_DISTANCE "Net/DoubleClickDistance"
#define XSETTINGS_REGISTRY_PROP_NET_DND_DRAG_THRESHOLD "Net/DndDragThreshold"
#define XSETTINGS_REGISTRY_PROP_NET_CURSOR_BLINK "Net/CursorBlink"
#define XSETTINGS_REGISTRY_PROP_NET_CURSOR_BLINK_TIME "Net/CursorBlinkTime"
#define XSETTINGS_REGISTRY_PROP_NET_THEME_NAME "Net/ThemeName"
#define XSETTINGS_REGISTRY_PROP_NET_ICON_THEME_NAME "Net/IconThemeName"
#define XSETTINGS_REGISTRY_PROP_NET_ENABLE_EVENT_SOUNDS "Net/EnableEventSounds"
#define XSETTINGS_REGISTRY_PROP_NET_SOUND_THEME_NAME "Net/SoundThemeName"
#define XSETTINGS_REGISTRY_PROP_NET_ENABLE_INPUT_FEEDBACK_SOUNDS "Net/EnableInputFeedbackSounds"
#define XSETTINGS_REGISTRY_PROP_NET_FALLBACK_ICON_THEME "Net/FallbackIconTheme"
#define XSETTINGS_REGISTRY_PROP_XFT_ANTIALIAS "Xft/Antialias"
#define XSETTINGS_REGISTRY_PROP_XFT_HINTING "Xft/Hinting"
#define XSETTINGS_REGISTRY_PROP_XFT_HINT_STYLE "Xft/HintStyle"
#define XSETTINGS_REGISTRY_PROP_XFT_RGBA "Xft/RGBA"
#define XSETTINGS_REGISTRY_PROP_XFT_DPI "Xft/DPI"
#define XSETTINGS_REGISTRY_PROP_XFT_LCDFILTER "Xft/lcdfilter"
#define XSETTINGS_REGISTRY_PROP_GTK_CURSOR_THEME_NAME "Gtk/CursorThemeName"
#define XSETTINGS_REGISTRY_PROP_GTK_CURSOR_THEME_SIZE "Gtk/CursorThemeSize"
#define XSETTINGS_REGISTRY_PROP_GTK_FONT_NAME "Gtk/FontName"
#define XSETTINGS_REGISTRY_PROP_GTK_KEY_THEME_NAME "Gtk/KeyThemeName"
#define XSETTINGS_REGISTRY_PROP_GTK_TOOLBAR_STYLE "Gtk/ToolbarStyle"
#define XSETTINGS_REGISTRY_PROP_GTK_TOOLBAR_ICON_SIZE "Gtk/ToolbarIconSize"
#define XSETTINGS_REGISTRY_PROP_GTK_IM_PREEDIT_STYLE "Gtk/IMPreeditStyle"
#define XSETTINGS_REGISTRY_PROP_GTK_IM_STATUS_STYLE "Gtk/IMStatusStyle"
#define XSETTINGS_REGISTRY_PROP_GTK_IM_MODULE "Gtk/IMModule"
#define XSETTINGS_REGISTRY_PROP_GTK_MENU_IMAGES "Gtk/MenuImages"
#define XSETTINGS_REGISTRY_PROP_GTK_BUTTON_IMAGES "Gtk/ButtonImages"
#define XSETTINGS_REGISTRY_PROP_GTK_MENU_BAR_ACCEL "Gtk/MenuBarAccel"
#define XSETTINGS_REGISTRY_PROP_GTK_COLOR_SCHEME "Gtk/ColorScheme"
#define XSETTINGS_REGISTRY_PROP_GTK_FILE_CHOOSER_BACKEND "Gtk/FileChooserBackend"
#define XSETTINGS_REGISTRY_PROP_GTK_DECORATION_LAYOUT "Gtk/DecorationLayout"
#define XSETTINGS_REGISTRY_PROP_GTK_SHELL_SHOWS_APP_MENU "Gtk/ShellShowsAppMenu"
#define XSETTINGS_REGISTRY_PROP_GTK_SHELL_SHOWS_MENUBAR "Gtk/ShellShowsMenubar"
#define XSETTINGS_REGISTRY_PROP_GTK_SHOW_INPUT_METHOD_MENU "Gtk/ShowInputMethodMenu"
#define XSETTINGS_REGISTRY_PROP_GTK_SHOW_UNICODE_MENU "Gtk/ShowUnicodeMenu"
#define XSETTINGS_REGISTRY_PROP_GTK_AUTO_MNEMONICS "Gtk/AutoMnemonics"
#define XSETTINGS_REGISTRY_PROP_GTK_ENABLE_PRIMARY_PASTE "Gtk/EnablePrimaryPaste"
#define XSETTINGS_REGISTRY_PROP_GTK_ENABLE_ANIMATIONS "Gtk/EnableAnimations"
#define XSETTINGS_REGISTRY_PROP_GTK_DIALOGS_USE_HEADER "Gtk/DialogsUseHeader"
#define XSETTINGS_REGISTRY_PROP_GDK_WINDOW_SCALING_FACTOR "Gdk/WindowScalingFactor"
#define XSETTINGS_REGISTRY_PROP_GDK_UNSCALED_DPI "Gdk/UnscaledDPI"
#define XSETTINGS_REGISTRY_PROP_FONTCONFIG_TIMESTAMP "Fontconfig/Timestamp"

#ifdef __cplusplus
}
#endif