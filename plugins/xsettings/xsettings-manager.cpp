/*
 * @Author       : tangjie02
 * @Date         : 2020-08-11 16:21:11
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-11-24 17:54:12
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/plugins/xsettings/xsettings-manager.cpp
 */

#include "plugins/xsettings/xsettings-manager.h"

#include "lib/display/EWMH.h"
#include "plugins/xsettings/xsettings-common.h"
#include "plugins/xsettings/xsettings-utils.h"

namespace Kiran
{
#define GSETTINGS_DBUS_NAME "com.kylinsec.Kiran.SessionDaemon.XSettings"
#define GSETTINGS_OBJECT_PATH "/com/kylinsec/Kiran/SessionDaemon/XSettings"

#define BACKGROUND_SCHAME_ID "org.mate.background"
#define BACKGROUND_SCHEMA_SHOW_DESKTOP_ICONS "show-desktop-icons"

const std::map<std::string, std::string> XSettingsManager::schema2registry_ =
    {
        {XSETTINGS_SCHEMA_DOUBLE_CLICK_TIME, "Net/DoubleClickTime"},
        {XSETTINGS_SCHEMA_DOUBLE_CLICK_DISTANCE, "Net/DoubleClickDistance"},
        {XSETTINGS_SCHEMA_DND_DRAG_THRESHOLD, "Net/DndDragThreshold"},
        {XSETTINGS_SCHEMA_CURSOR_BLINK, "Net/CursorBlink"},
        {XSETTINGS_SCHEMA_CURSOR_BLINK_TIME, "Net/CursorBlinkTime"},
        {XSETTINGS_SCHEMA_THEME_NAME, "Net/ThemeName"},
        {XSETTINGS_SCHEMA_XFT_ANTIALIAS, "Net/IconThemeName"},
        {XSETTINGS_SCHEMA_ICON_THEME_NAME, "Xft/Antialias"},
        {XSETTINGS_SCHEMA_XFT_HINTING, "Xft/Hinting"},
        {XSETTINGS_SCHEMA_XFT_HINT_STYLE, "Xft/HintStyle"},
        {XSETTINGS_SCHEMA_XFT_RGBA, "Xft/RGBA"},
        {XSETTINGS_SCHEMA_XFT_DPI, "Xft/DPI"},
        {XSETTINGS_SCHEMA_GTK_CURSOR_THEME_NAME, "Gtk/CursorThemeName"},
        {XSETTINGS_SCHEMA_GTK_CURSOR_THEME_SIZE, "Gtk/CursorThemeSize"},
        {XSETTINGS_SCHEMA_GTK_CAN_CHANGE_ACCELS, "Gtk/CanChangeAccels"},
        {XSETTINGS_SCHEMA_GTK_COLOR_PALETTE, "Gtk/ColorPalette"},
        {XSETTINGS_SCHEMA_GTK_FONT_NAME, "Gtk/FontName"},
        {XSETTINGS_SCHEMA_GTK_ICON_SIZES, "Gtk/IconSizes"},
        {XSETTINGS_SCHEMA_GTK_KEY_THEME_NAME, "Gtk/KeyThemeName"},
        {XSETTINGS_SCHEMA_GTK_TOOLBAR_STYLE, "Gtk/ToolbarStyle"},
        {XSETTINGS_SCHEMA_GTK_TOOLBAR_ICONS_SIZE, "Gtk/ToolbarIconSize"},
        {XSETTINGS_SCHEMA_GTK_IM_PREEDIT_STYLE, "Gtk/IMPreeditStyle"},
        {XSETTINGS_SCHEMA_GTK_IM_STATUS_STYLE, "Gtk/IMStatusStyle"},
        {XSETTINGS_SCHEMA_GTK_MENU_IMAGES, "Gtk/MenuImages"},
        {XSETTINGS_SCHEMA_GTK_BUTTON_IMAGES, "Gtk/ButtonImages"},
        {XSETTINGS_SCHEMA_GTK_MENUBAR_ACCEL, "Gtk/MenuBarAccel"},
        {XSETTINGS_SCHEMA_WINDOW_SCALING_FACTOR, "Gdk/WindowScalingFactor"},
};

// TODO: 设置初始值
XSettingsManager::XSettingsManager() : dbus_connect_id_(0),
                                       object_register_id_(0),
                                       window_scale_(0),
                                       double_click_time_(250)
{
    this->xsettings_settings_ = Gio::Settings::create(XSETTINGS_SCHEMA_ID);
    this->background_settings_ = Gio::Settings::create(BACKGROUND_SCHAME_ID);
}

XSettingsManager::~XSettingsManager()
{
    if (this->dbus_connect_id_)
    {
        Gio::DBus::unown_name(this->dbus_connect_id_);
    }
}

XSettingsManager *XSettingsManager::instance_ = nullptr;

void XSettingsManager::global_init()
{
    instance_ = new XSettingsManager();
    instance_->init();
}

int XSettingsManager::get_window_scale()
{
    auto scale = this->window_scaling_factor_get();
    if (!scale)
    {
        scale = XSettingsUtils::get_window_scale_auto();
    }
    return scale;
}

#define PROP_SET_HANDLER(prop, type1, key, type2)                                                                  \
    bool XSettingsManager::prop##_setHandler(type1 value)                                                          \
    {                                                                                                              \
        SETTINGS_PROFILE("value: %s.", fmt::format("{0}", value).c_str());                                         \
        RETURN_VAL_IF_TRUE(value == this->prop##_, false);                                                         \
        if (this->xsettings_settings_->get_##type2(key) != value)                                                  \
        {                                                                                                          \
            auto value_r = Glib::Variant<std::remove_cv<std::remove_reference<type1>::type>::type>::create(value); \
            if (!this->xsettings_settings_->set_value(key, value_r))                                               \
            {                                                                                                      \
                return false;                                                                                      \
            }                                                                                                      \
        }                                                                                                          \
        this->prop##_ = value;                                                                                     \
        return true;                                                                                               \
    }

PROP_SET_HANDLER(double_click_time, gint32, XSETTINGS_SCHEMA_DOUBLE_CLICK_TIME, int);
PROP_SET_HANDLER(double_click_distance, gint32, XSETTINGS_SCHEMA_DOUBLE_CLICK_DISTANCE, int);
PROP_SET_HANDLER(dnd_drag_threshold, gint32, XSETTINGS_SCHEMA_DND_DRAG_THRESHOLD, int);
PROP_SET_HANDLER(cursor_blink, bool, XSETTINGS_SCHEMA_CURSOR_BLINK, boolean);
PROP_SET_HANDLER(cursor_blink_time, gint32, XSETTINGS_SCHEMA_CURSOR_BLINK_TIME, int);
PROP_SET_HANDLER(theme_name, const Glib::ustring &, XSETTINGS_SCHEMA_THEME_NAME, string);
PROP_SET_HANDLER(xft_antialias, gint32, XSETTINGS_SCHEMA_XFT_ANTIALIAS, int);
PROP_SET_HANDLER(icon_theme_name, const Glib::ustring &, XSETTINGS_SCHEMA_ICON_THEME_NAME, string);
PROP_SET_HANDLER(xft_hinting, gint32, XSETTINGS_SCHEMA_XFT_HINTING, int);
PROP_SET_HANDLER(xft_hint_style, const Glib::ustring &, XSETTINGS_SCHEMA_XFT_HINT_STYLE, string);
PROP_SET_HANDLER(xft_rgba, const Glib::ustring &, XSETTINGS_SCHEMA_XFT_RGBA, string);
PROP_SET_HANDLER(xft_dpi, gint32, XSETTINGS_SCHEMA_XFT_DPI, int);
PROP_SET_HANDLER(gtk_cursor_theme_name, const Glib::ustring &, XSETTINGS_SCHEMA_GTK_CURSOR_THEME_NAME, string);
PROP_SET_HANDLER(gtk_cursor_theme_size, gint32, XSETTINGS_SCHEMA_GTK_CURSOR_THEME_SIZE, int);
PROP_SET_HANDLER(gtk_can_change_accels, bool, XSETTINGS_SCHEMA_GTK_CAN_CHANGE_ACCELS, boolean);
PROP_SET_HANDLER(gtk_color_palette, const Glib::ustring &, XSETTINGS_SCHEMA_GTK_COLOR_PALETTE, string);
PROP_SET_HANDLER(gtk_font_name, const Glib::ustring &, XSETTINGS_SCHEMA_GTK_FONT_NAME, string);
PROP_SET_HANDLER(gtk_icon_sizes, const Glib::ustring &, XSETTINGS_SCHEMA_GTK_ICON_SIZES, string);
PROP_SET_HANDLER(gtk_key_theme_name, const Glib::ustring &, XSETTINGS_SCHEMA_GTK_KEY_THEME_NAME, string);
PROP_SET_HANDLER(gtk_toolbar_style, const Glib::ustring &, XSETTINGS_SCHEMA_GTK_TOOLBAR_STYLE, string);
PROP_SET_HANDLER(gtk_toolbar_icons_size, const Glib::ustring &, XSETTINGS_SCHEMA_GTK_TOOLBAR_ICONS_SIZE, string);
PROP_SET_HANDLER(gtk_im_preedit_style, const Glib::ustring &, XSETTINGS_SCHEMA_GTK_IM_PREEDIT_STYLE, string);
PROP_SET_HANDLER(gtk_im_status_style, const Glib::ustring &, XSETTINGS_SCHEMA_GTK_IM_STATUS_STYLE, string);
PROP_SET_HANDLER(gtk_menu_images, bool, XSETTINGS_SCHEMA_GTK_MENU_IMAGES, boolean);
PROP_SET_HANDLER(gtk_button_images, bool, XSETTINGS_SCHEMA_GTK_BUTTON_IMAGES, boolean);
PROP_SET_HANDLER(gtk_menubar_accel, const Glib::ustring &, XSETTINGS_SCHEMA_GTK_MENUBAR_ACCEL, string);
PROP_SET_HANDLER(window_scaling_factor, gint32, XSETTINGS_SCHEMA_WINDOW_SCALING_FACTOR, int);
PROP_SET_HANDLER(window_scaling_factor_qt_sync, bool, XSETTINGS_SCHEMA_WINDOW_SCALING_FACTOR_QT_SYNC, boolean);

void XSettingsManager::init()
{
    RETURN_IF_FALSE(this->xsettings_settings_);
    RETURN_IF_FALSE(this->registry_.init());

    this->load_from_settings();

    this->xsettings_settings_->signal_changed().connect(sigc::bind(sigc::mem_fun(this, &XSettingsManager::settings_changed), true));

    auto screen = Gdk::Screen::get_default();
    screen->signal_size_changed().connect(sigc::mem_fun(this, &XSettingsManager::on_screen_changed));
    screen->signal_monitors_changed().connect(sigc::mem_fun(this, &XSettingsManager::on_screen_changed));

    this->dbus_connect_id_ = Gio::DBus::own_name(Gio::DBus::BUS_TYPE_SESSION,
                                                 GSETTINGS_DBUS_NAME,
                                                 sigc::mem_fun(this, &XSettingsManager::on_bus_acquired),
                                                 sigc::mem_fun(this, &XSettingsManager::on_name_acquired),
                                                 sigc::mem_fun(this, &XSettingsManager::on_name_lost));
}

void XSettingsManager::load_from_settings()
{
    SETTINGS_PROFILE("");

    for (const auto &key : this->xsettings_settings_->list_keys())
    {
        // 这里不做通知，等初始化完后统一通知
        this->settings_changed(key, false);
    }
    // 这里统一通知
    this->registry_.notify();
}

void XSettingsManager::settings_changed(const Glib::ustring &key, bool is_notify)
{
    SETTINGS_PROFILE("key: %s.", key.c_str());

    auto iter = this->schema2registry_.find(key);

#define SET_CASE(lower_prop, upper_prop, type, is_update)        \
    case CONNECT(XSETTINGS_SCHEMA_##upper_prop, _hash):          \
    {                                                            \
        auto value = this->xsettings_settings_->get_##type(key); \
        this->lower_prop##_set(value);                           \
        if (is_update && iter != this->schema2registry_.end())   \
        {                                                        \
            this->registry_.update(iter->second, value);         \
        }                                                        \
        break;                                                   \
    }

    switch (shash(key.c_str()))
    {
        SET_CASE(double_click_time, DOUBLE_CLICK_TIME, int, true);
        SET_CASE(double_click_distance, DOUBLE_CLICK_DISTANCE, int, true);
        SET_CASE(dnd_drag_threshold, DND_DRAG_THRESHOLD, int, true);
        SET_CASE(cursor_blink, CURSOR_BLINK, boolean, true);
        SET_CASE(cursor_blink_time, CURSOR_BLINK_TIME, int, true);
        SET_CASE(theme_name, THEME_NAME, string, true);
        SET_CASE(xft_antialias, XFT_ANTIALIAS, int, true);
        SET_CASE(icon_theme_name, ICON_THEME_NAME, string, true);
        SET_CASE(xft_hinting, XFT_HINTING, int, true);
        SET_CASE(xft_hint_style, XFT_HINT_STYLE, string, true);
        SET_CASE(xft_rgba, XFT_RGBA, string, true);
        SET_CASE(gtk_cursor_theme_name, GTK_CURSOR_THEME_NAME, string, true);
        SET_CASE(gtk_cursor_theme_size, GTK_CURSOR_THEME_SIZE, int, false);
        SET_CASE(gtk_can_change_accels, GTK_CAN_CHANGE_ACCELS, boolean, true);
        SET_CASE(gtk_color_palette, GTK_COLOR_PALETTE, string, true);
        SET_CASE(gtk_font_name, GTK_FONT_NAME, string, true);
        SET_CASE(gtk_icon_sizes, GTK_ICON_SIZES, string, true);
        SET_CASE(gtk_key_theme_name, GTK_KEY_THEME_NAME, string, true);
        SET_CASE(gtk_toolbar_style, GTK_TOOLBAR_STYLE, string, true);
        SET_CASE(gtk_toolbar_icons_size, GTK_TOOLBAR_ICONS_SIZE, string, true);
        SET_CASE(gtk_im_preedit_style, GTK_IM_PREEDIT_STYLE, string, true);
        SET_CASE(gtk_im_status_style, GTK_IM_STATUS_STYLE, string, true);
        SET_CASE(gtk_menu_images, GTK_MENU_IMAGES, boolean, true);
        SET_CASE(gtk_button_images, GTK_BUTTON_IMAGES, boolean, true);
        SET_CASE(gtk_menubar_accel, GTK_MENUBAR_ACCEL, string, true);
        SET_CASE(window_scaling_factor, WINDOW_SCALING_FACTOR, int, false);
        SET_CASE(window_scaling_factor_qt_sync, WINDOW_SCALING_FACTOR_QT_SYNC, int, false);

    // Ignore these properties
    case CONNECT(XSETTINGS_SCHEMA_XFT_DPI, _hash):
        break;

    default:
        LOG_WARNING("Unknown key: %s.", key.c_str());
        break;
    }
#undef SET_CASET

    switch (shash(key.c_str()))
    {
    case CONNECT(XSETTINGS_SCHEMA_WINDOW_SCALING_FACTOR, _hash):
    case CONNECT(XSETTINGS_SCHEMA_GTK_CURSOR_THEME_SIZE, _hash):
        this->scale_settings();
        break;
    }

    this->xsettings_changed_.emit(key.raw());

    if (is_notify)
    {
        this->registry_.notify();
    }
}

void XSettingsManager::scale_settings()
{
    auto scale = this->get_window_scale();
    auto dpi = XSettingsUtils::get_dpi_from_x_server();
    int32_t unscaled_dpi = int32_t(dpi * 1024);
    int32_t scaled_dpi = int32_t(CLAMP(dpi * scale, DPI_LOW_REASONABLE_VALUE, DPI_HIGH_REASONABLE_VALUE) * 1024);
    auto scaled_cursor_size = this->gtk_cursor_theme_size_get() * scale;

    this->registry_.update("Gdk/WindowScalingFactor", scale);
    this->registry_.update("Gdk/UnscaledDPI", unscaled_dpi);
    this->registry_.update("Xft/DPI", scaled_dpi);
    this->registry_.update("Gtk/CursorThemeSize", scaled_cursor_size);

    this->xft_dpi_set(scaled_dpi);
    this->scale_change_workarounds(scale);
}

void XSettingsManager::scale_change_workarounds(int32_t scale)
{
    std::string error;
    RETURN_IF_TRUE(this->window_scale_ == scale);
    // 第一次初始化时设置
    if (!this->window_scale_)
    {
        // 如果开启QT缩放同步，则将缩放值同步到QT缩放相关的环境变量
        if (this->window_scaling_factor_qt_sync_get())
        {
            if (!XSettingsUtils::update_user_env_variable("QT_AUTO_SCREEN_SCALE_FACTOR", "0", error))
            {
                LOG_WARNING("There was a problem when setting QT_AUTO_SCREEN_SCALE_FACTOR=0: %s", error.c_str());
            }

            if (!XSettingsUtils::update_user_env_variable("QT_SCALE_FACTOR", scale == 2 ? "2" : "1", error))
            {
                LOG_WARNING("There was a problem when setting QT_SCALE_FACTOR=%d: %s", scale, error.c_str());
            }
        }
    }
    else
    {
        // 理想的情况是marco/mate-panel/caja监控缩放因子的变化而自动调整自己的大小，
        // 但实际上没有实现这个功能，所以当窗口缩放因子发生变化时重置它们

        // 重启marco窗口管理器
        auto wm_name = EWMH::get_instance()->get_wm_name();
        if (wm_name == WM_COMMON_MARCO)
        {
            std::vector<std::string> argv = {"marco", "--replace"};

            try
            {
                Glib::spawn_async(std::string(), argv, Glib::SPAWN_SEARCH_PATH);
            }
            catch (const Glib::Error &e)
            {
                LOG_WARNING("There was a problem restarting marco: %s", e.what().c_str());
            }
        }
        // 重启面板
        std::vector<std::string> argv = {"killall", "mate-panel"};
        try
        {
            Glib::spawn_async(std::string(), argv, Glib::SPAWN_SEARCH_PATH);
        }
        catch (const Glib::Error &e)
        {
            LOG_WARNING("There was a problem restarting mate-panel: %s", e.what().c_str());
        }

        // 重置桌面图标大小
        if (this->background_settings_ &&
            this->background_settings_->get_boolean(BACKGROUND_SCHEMA_SHOW_DESKTOP_ICONS))
        {
            // 延时隐藏/显示桌面图标，给文件管理器一定的时间重绘
            auto timeout = Glib::MainContext::get_default()->signal_timeout();
            timeout.connect_seconds(sigc::bind(sigc::mem_fun(this, &XSettingsManager::delayed_toggle_bg_draw), false), 1);
            timeout.connect_seconds(sigc::bind(sigc::mem_fun(this, &XSettingsManager::delayed_toggle_bg_draw), true), 2);
        }
    }

    this->window_scale_ = scale;
}

void XSettingsManager::on_screen_changed()
{
    auto scale = this->get_window_scale();
    if (scale != this->window_scale_)
    {
        this->scale_settings();
    }
    this->registry_.notify();
}

bool XSettingsManager::delayed_toggle_bg_draw(bool value)
{
    if (this->background_settings_)
    {
        this->background_settings_->set_boolean(BACKGROUND_SCHEMA_SHOW_DESKTOP_ICONS, value);
    }
    return false;
}

void XSettingsManager::on_bus_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name)
{
    SETTINGS_PROFILE("name: %s", name.c_str());
    if (!connect)
    {
        LOG_WARNING("failed to connect dbus. name: %s", name.c_str());
        return;
    }
    try
    {
        this->object_register_id_ = this->register_object(connect, GSETTINGS_OBJECT_PATH);
    }
    catch (const Glib::Error &e)
    {
        LOG_WARNING("register object_path %s fail: %s.", GSETTINGS_OBJECT_PATH, e.what().c_str());
    }
}

void XSettingsManager::on_name_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name)
{
    LOG_DEBUG("success to register dbus name: %s", name.c_str());
}

void XSettingsManager::on_name_lost(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name)
{
    LOG_WARNING("failed to register dbus name: %s", name.c_str());
}

}  // namespace Kiran