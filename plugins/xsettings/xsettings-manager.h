/*
 * @Author       : tangjie02
 * @Date         : 2020-08-11 16:21:04
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-11-24 17:49:26
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/plugins/xsettings/xsettings-manager.h
 */

#pragma once

#include <xsettings_dbus_stub.h>

#include "plugins/xsettings/xsettings-registry.h"

namespace Kiran
{
class XSettingsManager : public SessionDaemon::XSettingsStub
{
public:
    XSettingsManager();
    virtual ~XSettingsManager();

    static XSettingsManager *get_instance() { return instance_; };

    static void global_init();

    static void global_deinit() { delete instance_; };

    sigc::signal<void, const std::string &> &signal_xsettings_changed() { return this->xsettings_changed_; };

    int get_window_scale();

public:
    virtual gint32 double_click_time_get() { return this->double_click_time_; }
    virtual gint32 double_click_distance_get() { return this->double_click_distance_; }
    virtual gint32 dnd_drag_threshold_get() { return this->dnd_drag_threshold_; }
    virtual bool cursor_blink_get() { return this->cursor_blink_; }
    virtual gint32 cursor_blink_time_get() { return this->cursor_blink_time_; }
    virtual Glib::ustring theme_name_get() { return this->theme_name_; }
    virtual gint32 xft_antialias_get() { return this->xft_antialias_; }
    virtual Glib::ustring icon_theme_name_get() { return this->icon_theme_name_; }
    virtual gint32 xft_hinting_get() { return this->xft_hinting_; }
    virtual Glib::ustring xft_hint_style_get() { return this->xft_hint_style_; }
    virtual Glib::ustring xft_rgba_get() { return this->xft_rgba_; }
    virtual gint32 xft_dpi_get() { return this->xft_dpi_; }
    virtual Glib::ustring gtk_cursor_theme_name_get() { return this->gtk_cursor_theme_name_; }
    virtual gint32 gtk_cursor_theme_size_get() { return this->gtk_cursor_theme_size_; }
    virtual bool gtk_can_change_accels_get() { return this->gtk_can_change_accels_; }
    virtual Glib::ustring gtk_color_palette_get() { return this->gtk_color_palette_; }
    virtual Glib::ustring gtk_font_name_get() { return this->gtk_font_name_; }
    virtual Glib::ustring gtk_icon_sizes_get() { return this->gtk_icon_sizes_; }
    virtual Glib::ustring gtk_key_theme_name_get() { return this->gtk_key_theme_name_; }
    virtual Glib::ustring gtk_toolbar_style_get() { return this->gtk_toolbar_style_; }
    virtual Glib::ustring gtk_toolbar_icons_size_get() { return this->gtk_toolbar_icons_size_; }
    virtual Glib::ustring gtk_im_preedit_style_get() { return this->gtk_im_preedit_style_; }
    virtual Glib::ustring gtk_im_status_style_get() { return this->gtk_im_status_style_; }
    virtual bool gtk_menu_images_get() { return this->gtk_menu_images_; }
    virtual bool gtk_button_images_get() { return this->gtk_button_images_; }
    virtual Glib::ustring gtk_menubar_accel_get() { return this->gtk_menubar_accel_; }
    virtual gint32 window_scaling_factor_get() { return this->window_scaling_factor_; }
    virtual bool window_scaling_factor_qt_sync_get() { return this->window_scaling_factor_qt_sync_; }

protected:
    virtual bool double_click_time_setHandler(gint32 value);
    virtual bool double_click_distance_setHandler(gint32 value);
    virtual bool dnd_drag_threshold_setHandler(gint32 value);
    virtual bool cursor_blink_setHandler(bool value);
    virtual bool cursor_blink_time_setHandler(gint32 value);
    virtual bool theme_name_setHandler(const Glib::ustring &value);
    virtual bool icon_theme_name_setHandler(const Glib::ustring &value);
    virtual bool xft_antialias_setHandler(gint32 value);
    virtual bool xft_hinting_setHandler(gint32 value);
    virtual bool xft_hint_style_setHandler(const Glib::ustring &value);
    virtual bool xft_rgba_setHandler(const Glib::ustring &value);
    virtual bool xft_dpi_setHandler(gint32 value);
    virtual bool gtk_cursor_theme_name_setHandler(const Glib::ustring &value);
    virtual bool gtk_cursor_theme_size_setHandler(gint32 value);
    virtual bool gtk_can_change_accels_setHandler(bool value);
    virtual bool gtk_color_palette_setHandler(const Glib::ustring &value);
    virtual bool gtk_font_name_setHandler(const Glib::ustring &value);
    virtual bool gtk_icon_sizes_setHandler(const Glib::ustring &value);
    virtual bool gtk_key_theme_name_setHandler(const Glib::ustring &value);
    virtual bool gtk_toolbar_style_setHandler(const Glib::ustring &value);
    virtual bool gtk_toolbar_icons_size_setHandler(const Glib::ustring &value);
    virtual bool gtk_im_preedit_style_setHandler(const Glib::ustring &value);
    virtual bool gtk_im_status_style_setHandler(const Glib::ustring &value);
    virtual bool gtk_menu_images_setHandler(bool value);
    virtual bool gtk_button_images_setHandler(bool value);
    virtual bool gtk_menubar_accel_setHandler(const Glib::ustring &value);
    virtual bool window_scaling_factor_setHandler(gint32 value);
    virtual bool window_scaling_factor_qt_sync_setHandler(bool value);

private:
    void init();

    void load_from_settings();
    void settings_changed(const Glib::ustring &key, bool is_notify);
    void scale_settings();
    void scale_change_workarounds(int32_t scale);
    void on_screen_changed();
    bool delayed_toggle_bg_draw(bool value);

    void on_bus_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name);
    void on_name_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name);
    void on_name_lost(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name);

private:
    static XSettingsManager *instance_;

    uint32_t dbus_connect_id_;
    uint32_t object_register_id_;

    sigc::signal<void, const std::string &> xsettings_changed_;
    // 当window_scaling_factor_为0时，根据屏幕信息设置缩放，否则跟window_scaling_factor_相同。
    int32_t window_scale_;

    Glib::RefPtr<Gio::Settings> xsettings_settings_;
    Glib::RefPtr<Gio::Settings> background_settings_;
    XSettingsRegistry registry_;

    const static std::map<std::string, std::string> schema2registry_;

    int32_t double_click_time_;
    int32_t double_click_distance_;
    int32_t dnd_drag_threshold_;
    bool cursor_blink_;
    int32_t cursor_blink_time_;
    Glib::ustring theme_name_;
    gint32 xft_antialias_;
    Glib::ustring icon_theme_name_;
    int32_t xft_hinting_;
    Glib::ustring xft_hint_style_;
    Glib::ustring xft_rgba_;
    int32_t xft_dpi_;
    Glib::ustring gtk_cursor_theme_name_;
    int32_t gtk_cursor_theme_size_;
    bool gtk_can_change_accels_;
    Glib::ustring gtk_color_palette_;
    Glib::ustring gtk_font_name_;
    Glib::ustring gtk_icon_sizes_;
    Glib::ustring gtk_key_theme_name_;
    Glib::ustring gtk_toolbar_style_;
    Glib::ustring gtk_toolbar_icons_size_;
    Glib::ustring gtk_im_preedit_style_;
    Glib::ustring gtk_im_status_style_;
    bool gtk_menu_images_;
    bool gtk_button_images_;
    Glib::ustring gtk_menubar_accel_;
    int32_t window_scaling_factor_;
    bool window_scaling_factor_qt_sync_;
};
}  // namespace Kiran