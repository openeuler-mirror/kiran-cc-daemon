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

#include <xsettings_dbus_stub.h>

#include "plugins/xsettings/fontconfig-monitor.h"
#include "plugins/xsettings/xsettings-common.h"
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
    virtual void ListPropertyNames(MethodInvocation &invocation);
    virtual void GetInteger(const Glib::ustring &name, MethodInvocation &invocation);
    virtual void SetInteger(const Glib::ustring &name, gint32 value, MethodInvocation &invocation);
    virtual void GetString(const Glib::ustring &name, MethodInvocation &invocation);
    virtual void SetString(const Glib::ustring &name, const Glib::ustring &value, MethodInvocation &invocation);
    virtual void GetColor(const Glib::ustring &name, MethodInvocation &invocation);
    virtual void SetColor(const Glib::ustring &name, const std::tuple<guint16, guint16, guint16, guint16> &value, MethodInvocation &invocation);

    int32_t get_xft_antialias() { return this->xsettings_settings_->get_int(XSETTINGS_SCHEMA_XFT_ANTIALIAS); }
    int32_t get_xft_hinting() { return this->xsettings_settings_->get_int(XSETTINGS_SCHEMA_XFT_HINTING); }
    std::string get_xft_hint_style() { return this->xsettings_settings_->get_string(XSETTINGS_SCHEMA_XFT_HINT_STYLE); }
    std::string get_xft_rgba() { return this->xsettings_settings_->get_string(XSETTINGS_SCHEMA_XFT_RGBA); }
    int32_t get_xft_dpi() { return this->xsettings_settings_->get_int(XSETTINGS_SCHEMA_XFT_DPI); }
    std::string get_gtk_cursor_theme_name() { return this->xsettings_settings_->get_string(XSETTINGS_SCHEMA_GTK_CURSOR_THEME_NAME); }
    int32_t get_gtk_cursor_theme_size() { return this->xsettings_settings_->get_int(XSETTINGS_SCHEMA_GTK_CURSOR_THEME_SIZE); }
    int32_t get_window_scaling_factor() { return this->xsettings_settings_->get_int(XSETTINGS_SCHEMA_WINDOW_SCALING_FACTOR); }
    bool get_window_scaling_factor_qt_sync() { return this->xsettings_settings_->get_boolean(XSETTINGS_SCHEMA_WINDOW_SCALING_FACTOR_QT_SYNC); }

private:
    void init();

    void load_from_settings();
    void settings_changed(const Glib::ustring &key, bool is_notify);
    void scale_settings();
    void scale_change_workarounds(int32_t scale);
    void on_screen_changed();
    bool delayed_toggle_bg_draw(bool value);
    void on_fontconfig_timestamp_changed();

    void set_registry_var(std::shared_ptr<XSettingsPropertyBase> var, MethodInvocation &invocation);

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
    std::map<std::string, std::string> registry2schema_;

    FontconfigMonitor fontconfig_monitor_;
};
}  // namespace Kiran