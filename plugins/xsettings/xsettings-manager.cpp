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

#include "plugins/xsettings/xsettings-manager.h"

#include "lib/display/EWMH.h"
#include "plugins/xsettings/xsettings-utils.h"

namespace Kiran
{
#define BACKGROUND_SCHAME_ID "org.mate.background"
#define BACKGROUND_SCHEMA_SHOW_DESKTOP_ICONS "show-desktop-icons"

const std::map<std::string, std::string> XSettingsManager::schema2registry_ =
    {
        {XSETTINGS_SCHEMA_NET_DOUBLE_CLICK_TIME, XSETTINGS_REGISTRY_PROP_NET_DOUBLE_CLICK_TIME},
        {XSETTINGS_SCHEMA_NET_DOUBLE_CLICK_DISTANCE, XSETTINGS_REGISTRY_PROP_NET_DOUBLE_CLICK_DISTANCE},
        {XSETTINGS_SCHEMA_NET_DND_DRAG_THRESHOLD, XSETTINGS_REGISTRY_PROP_NET_DND_DRAG_THRESHOLD},
        {XSETTINGS_SCHEMA_NET_CURSOR_BLINK, XSETTINGS_REGISTRY_PROP_NET_CURSOR_BLINK},
        {XSETTINGS_SCHEMA_NET_CURSOR_BLINK_TIME, XSETTINGS_REGISTRY_PROP_NET_CURSOR_BLINK_TIME},
        {XSETTINGS_SCHEMA_NET_THEME_NAME, XSETTINGS_REGISTRY_PROP_NET_THEME_NAME},
        {XSETTINGS_SCHEMA_NET_ICON_THEME_NAME, XSETTINGS_REGISTRY_PROP_NET_ICON_THEME_NAME},
        {XSETTINGS_SCHEMA_NET_ENABLE_EVENT_SOUNDS, XSETTINGS_REGISTRY_PROP_NET_ENABLE_EVENT_SOUNDS},
        {XSETTINGS_SCHEMA_NET_SOUND_THEME_NAME, XSETTINGS_REGISTRY_PROP_NET_SOUND_THEME_NAME},
        {XSETTINGS_SCHEMA_NET_ENABLE_INPUT_FEEDBACK_SOUNDS, XSETTINGS_REGISTRY_PROP_NET_ENABLE_INPUT_FEEDBACK_SOUNDS},

        {XSETTINGS_SCHEMA_XFT_ANTIALIAS, XSETTINGS_REGISTRY_PROP_XFT_ANTIALIAS},
        {XSETTINGS_SCHEMA_XFT_HINTING, XSETTINGS_REGISTRY_PROP_XFT_HINTING},
        {XSETTINGS_SCHEMA_XFT_HINT_STYLE, XSETTINGS_REGISTRY_PROP_XFT_HINT_STYLE},
        {XSETTINGS_SCHEMA_XFT_RGBA, XSETTINGS_REGISTRY_PROP_XFT_RGBA},
        {XSETTINGS_SCHEMA_XFT_DPI, XSETTINGS_REGISTRY_PROP_XFT_DPI},

        {XSETTINGS_SCHEMA_GTK_CURSOR_THEME_NAME, XSETTINGS_REGISTRY_PROP_GTK_CURSOR_THEME_NAME},
        {XSETTINGS_SCHEMA_GTK_CURSOR_THEME_SIZE, XSETTINGS_REGISTRY_PROP_GTK_CURSOR_THEME_SIZE},
        {XSETTINGS_SCHEMA_GTK_FONT_NAME, XSETTINGS_REGISTRY_PROP_GTK_FONT_NAME},
        {XSETTINGS_SCHEMA_GTK_KEY_THEME_NAME, XSETTINGS_REGISTRY_PROP_GTK_KEY_THEME_NAME},
        {XSETTINGS_SCHEMA_GTK_TOOLBAR_STYLE, XSETTINGS_REGISTRY_PROP_GTK_TOOLBAR_STYLE},
        {XSETTINGS_SCHEMA_GTK_TOOLBAR_ICONS_SIZE, XSETTINGS_REGISTRY_PROP_GTK_TOOLBAR_ICON_SIZE},
        {XSETTINGS_SCHEMA_GTK_IM_PREEDIT_STYLE, XSETTINGS_REGISTRY_PROP_GTK_IM_PREEDIT_STYLE},
        {XSETTINGS_SCHEMA_GTK_IM_STATUS_STYLE, XSETTINGS_REGISTRY_PROP_GTK_IM_STATUS_STYLE},
        {XSETTINGS_SCHEMA_GTK_IM_MODULE, XSETTINGS_REGISTRY_PROP_GTK_IM_MODULE},
        {XSETTINGS_SCHEMA_GTK_MENU_IMAGES, XSETTINGS_REGISTRY_PROP_GTK_MENU_IMAGES},
        {XSETTINGS_SCHEMA_GTK_BUTTON_IMAGES, XSETTINGS_REGISTRY_PROP_GTK_BUTTON_IMAGES},
        {XSETTINGS_SCHEMA_GTK_MENUBAR_ACCEL, XSETTINGS_REGISTRY_PROP_GTK_MENU_BAR_ACCEL},
        {XSETTINGS_SCHEMA_GTK_COLOR_SCHEME, XSETTINGS_REGISTRY_PROP_GTK_COLOR_SCHEME},
        {XSETTINGS_SCHEMA_GTK_FILE_CHOOSER_BACKEND, XSETTINGS_REGISTRY_PROP_GTK_FILE_CHOOSER_BACKEND},
        {XSETTINGS_SCHEMA_GTK_DECORATION_LAYOUT, XSETTINGS_REGISTRY_PROP_GTK_DECORATION_LAYOUT},
        {XSETTINGS_SCHEMA_GTK_SHELL_SHOWS_APP_MENU, XSETTINGS_REGISTRY_PROP_GTK_SHELL_SHOWS_APP_MENU},
        {XSETTINGS_SCHEMA_GTK_SHELL_SHOWS_MENUBAR, XSETTINGS_REGISTRY_PROP_GTK_SHELL_SHOWS_MENUBAR},
        {XSETTINGS_SCHEMA_GTK_SHOW_INPUT_METHOD_MENU, XSETTINGS_REGISTRY_PROP_GTK_SHOW_INPUT_METHOD_MENU},
        {XSETTINGS_SCHEMA_GTK_SHOW_UNICODE_MENU, XSETTINGS_REGISTRY_PROP_GTK_SHOW_UNICODE_MENU},
        {XSETTINGS_SCHEMA_GTK_AUTOMATIC_MNEMONICS, XSETTINGS_REGISTRY_PROP_GTK_AUTO_MNEMONICS},
        {XSETTINGS_SCHEMA_GTK_ENABLE_PRIMARY_PASTE, XSETTINGS_REGISTRY_PROP_GTK_ENABLE_PRIMARY_PASTE},
        {XSETTINGS_SCHEMA_GTK_ENABLE_ANIMATIONS, XSETTINGS_REGISTRY_PROP_GTK_ENABLE_ANIMATIONS},
        {XSETTINGS_SCHEMA_GTK_DIALOGS_USE_HEADER, XSETTINGS_REGISTRY_PROP_GTK_DIALOGS_USE_HEADER},

        {XSETTINGS_SCHEMA_WINDOW_SCALING_FACTOR, XSETTINGS_REGISTRY_PROP_GDK_WINDOW_SCALING_FACTOR},
};

// TODO: 设置初始值
XSettingsManager::XSettingsManager() : dbus_connect_id_(0),
                                       object_register_id_(0),
                                       window_scale_(0)
{
    this->xsettings_settings_ = Gio::Settings::create(XSETTINGS_SCHEMA_ID);
    this->background_settings_ = Gio::Settings::create(BACKGROUND_SCHAME_ID);

    for (const auto &iter : XSettingsManager::schema2registry_)
    {
        this->registry2schema_.emplace(iter.second, iter.first);
    }
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
    auto scale = this->get_window_scaling_factor();
    if (!scale)
    {
        scale = XSettingsUtils::get_window_scale_auto();
    }
    return scale;
}

#define CHECK_VAR(var, type)                                                           \
    if (!var)                                                                          \
    {                                                                                  \
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_XSETTINGS_NOTFOUND_PROPERTY);      \
    }                                                                                  \
    if (var->get_type() != type)                                                       \
    {                                                                                  \
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_XSETTINGS_PROPERTY_TYPE_MISMATCH); \
    }

void XSettingsManager::ListPropertyNames(MethodInvocation &invocation)
{
    std::vector<Glib::ustring> property_names;
    auto properties = this->registry_.get_properties();
    for (auto &iter : properties)
    {
        property_names.push_back(iter->get_name());
    }
    invocation.ret(property_names);
}

void XSettingsManager::GetInteger(const Glib::ustring &name, MethodInvocation &invocation)
{
    auto var = this->registry_.get_property(name);
    CHECK_VAR(var, XSettingsPropType::XSETTINGS_PROP_TYPE_INT);

    auto int_var = std::dynamic_pointer_cast<XSettingsPropertyInt>(var);
    invocation.ret(int_var->get_value());
}

void XSettingsManager::SetInteger(const Glib::ustring &name, gint32 value, MethodInvocation &invocation)
{
    auto var = std::make_shared<XSettingsPropertyInt>(name, value);
    this->set_registry_var(var, invocation);
}

void XSettingsManager::GetString(const Glib::ustring &name, MethodInvocation &invocation)
{
    auto var = this->registry_.get_property(name);
    CHECK_VAR(var, XSettingsPropType::XSETTINGS_PROP_TYPE_STRING);

    auto string_var = std::dynamic_pointer_cast<XSettingsPropertyString>(var);
    invocation.ret(string_var->get_value());
}

void XSettingsManager::SetString(const Glib::ustring &name, const Glib::ustring &value, MethodInvocation &invocation)
{
    auto var = std::make_shared<XSettingsPropertyString>(name, value);
    this->set_registry_var(var, invocation);
}

void XSettingsManager::GetColor(const Glib::ustring &name, MethodInvocation &invocation)
{
    auto var = this->registry_.get_property(name);
    CHECK_VAR(var, XSettingsPropType::XSETTINGS_PROP_TYPE_COLOR);

    auto color_var = std::dynamic_pointer_cast<XSettingsPropertyColor>(var);
    auto color_value = color_var->get_value();
    invocation.ret(std::make_tuple(color_value.red, color_value.green, color_value.blue, color_value.alpha));
}

void XSettingsManager::SetColor(const Glib::ustring &name, const std::tuple<guint16, guint16, guint16, guint16> &value, MethodInvocation &invocation)
{
    XSettingsColor color_value = {std::get<0>(value), std::get<1>(value), std::get<2>(value), std::get<3>(value)};
    auto var = std::make_shared<XSettingsPropertyColor>(name, color_value);
    this->set_registry_var(var, invocation);
}

void XSettingsManager::init()
{
    RETURN_IF_FALSE(this->xsettings_settings_);
    RETURN_IF_FALSE(this->registry_.init());
    this->fontconfig_monitor_.init();

    this->load_from_settings();

    this->xsettings_settings_->signal_changed().connect(sigc::bind(sigc::mem_fun(this, &XSettingsManager::settings_changed), true));
    auto screen = Gdk::Screen::get_default();
    screen->signal_size_changed().connect(sigc::mem_fun(this, &XSettingsManager::on_screen_changed));
    screen->signal_monitors_changed().connect(sigc::mem_fun(this, &XSettingsManager::on_screen_changed));
    this->fontconfig_monitor_.signal_timestamp_changed().connect(sigc::mem_fun(this, &XSettingsManager::on_fontconfig_timestamp_changed));

    this->dbus_connect_id_ = Gio::DBus::own_name(Gio::DBus::BUS_TYPE_SESSION,
                                                 XSETTINGS_DBUS_NAME,
                                                 sigc::mem_fun(this, &XSettingsManager::on_bus_acquired),
                                                 sigc::mem_fun(this, &XSettingsManager::on_name_acquired),
                                                 sigc::mem_fun(this, &XSettingsManager::on_name_lost));
}

void XSettingsManager::load_from_settings()
{
    KLOG_PROFILE("");

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
    if (is_notify)
    {
        KLOG_DEBUG("key: %s.", key.c_str());
    }

    auto iter = this->schema2registry_.find(key);

#define SET_CASE(prop, type)                                     \
    case CONNECT(prop, _hash):                                   \
    {                                                            \
        auto value = this->xsettings_settings_->get_##type(key); \
        this->registry_.update(iter->second, value);             \
        break;                                                   \
    }

    switch (shash(key.c_str()))
    {
        SET_CASE(XSETTINGS_SCHEMA_NET_DOUBLE_CLICK_TIME, int);
        SET_CASE(XSETTINGS_SCHEMA_NET_DOUBLE_CLICK_DISTANCE, int);
        SET_CASE(XSETTINGS_SCHEMA_NET_DND_DRAG_THRESHOLD, int);
        SET_CASE(XSETTINGS_SCHEMA_NET_CURSOR_BLINK, boolean);
        SET_CASE(XSETTINGS_SCHEMA_NET_CURSOR_BLINK_TIME, int);
        SET_CASE(XSETTINGS_SCHEMA_NET_THEME_NAME, string);
        SET_CASE(XSETTINGS_SCHEMA_NET_ICON_THEME_NAME, string);
        SET_CASE(XSETTINGS_SCHEMA_NET_ENABLE_EVENT_SOUNDS, boolean);
        SET_CASE(XSETTINGS_SCHEMA_NET_SOUND_THEME_NAME, string);
        SET_CASE(XSETTINGS_SCHEMA_NET_ENABLE_INPUT_FEEDBACK_SOUNDS, boolean);

        SET_CASE(XSETTINGS_SCHEMA_XFT_ANTIALIAS, int);
        SET_CASE(XSETTINGS_SCHEMA_XFT_HINTING, int);
        SET_CASE(XSETTINGS_SCHEMA_XFT_HINT_STYLE, string);
        SET_CASE(XSETTINGS_SCHEMA_XFT_RGBA, string);

        SET_CASE(XSETTINGS_SCHEMA_GTK_CURSOR_THEME_NAME, string);
        SET_CASE(XSETTINGS_SCHEMA_GTK_FONT_NAME, string);
        SET_CASE(XSETTINGS_SCHEMA_GTK_KEY_THEME_NAME, string);
        SET_CASE(XSETTINGS_SCHEMA_GTK_TOOLBAR_STYLE, string);
        SET_CASE(XSETTINGS_SCHEMA_GTK_TOOLBAR_ICONS_SIZE, string);
        SET_CASE(XSETTINGS_SCHEMA_GTK_IM_PREEDIT_STYLE, string);
        SET_CASE(XSETTINGS_SCHEMA_GTK_IM_STATUS_STYLE, string);
        SET_CASE(XSETTINGS_SCHEMA_GTK_IM_MODULE, string);
        SET_CASE(XSETTINGS_SCHEMA_GTK_MENU_IMAGES, boolean);
        SET_CASE(XSETTINGS_SCHEMA_GTK_BUTTON_IMAGES, boolean);
        SET_CASE(XSETTINGS_SCHEMA_GTK_MENUBAR_ACCEL, string);
        SET_CASE(XSETTINGS_SCHEMA_GTK_COLOR_SCHEME, string);
        SET_CASE(XSETTINGS_SCHEMA_GTK_FILE_CHOOSER_BACKEND, string);
        SET_CASE(XSETTINGS_SCHEMA_GTK_DECORATION_LAYOUT, string);
        SET_CASE(XSETTINGS_SCHEMA_GTK_SHELL_SHOWS_APP_MENU, boolean);
        SET_CASE(XSETTINGS_SCHEMA_GTK_SHELL_SHOWS_MENUBAR, boolean);
        SET_CASE(XSETTINGS_SCHEMA_GTK_SHOW_INPUT_METHOD_MENU, boolean);
        SET_CASE(XSETTINGS_SCHEMA_GTK_SHOW_UNICODE_MENU, boolean);
        SET_CASE(XSETTINGS_SCHEMA_GTK_AUTOMATIC_MNEMONICS, boolean);
        SET_CASE(XSETTINGS_SCHEMA_GTK_ENABLE_PRIMARY_PASTE, boolean);
        SET_CASE(XSETTINGS_SCHEMA_GTK_ENABLE_ANIMATIONS, boolean);
        SET_CASE(XSETTINGS_SCHEMA_GTK_DIALOGS_USE_HEADER, boolean);

        // Ignore these properties
    case CONNECT(XSETTINGS_SCHEMA_GTK_CURSOR_THEME_SIZE, _hash):
    case CONNECT(XSETTINGS_SCHEMA_WINDOW_SCALING_FACTOR, _hash):
    case CONNECT(XSETTINGS_SCHEMA_WINDOW_SCALING_FACTOR_QT_SYNC, _hash):
    case CONNECT(XSETTINGS_SCHEMA_XFT_DPI, _hash):
        break;

    default:
        KLOG_WARNING("Unknown key: %s.", key.c_str());
        break;
    }
#undef SET_CASET

    switch (shash(key.c_str()))
    {
    case CONNECT(XSETTINGS_SCHEMA_WINDOW_SCALING_FACTOR, _hash):
    case CONNECT(XSETTINGS_SCHEMA_GTK_CURSOR_THEME_SIZE, _hash):
        this->scale_settings();
        break;
    case CONNECT(XSETTINGS_SCHEMA_XFT_RGBA, _hash):
        this->registry_.update(XSETTINGS_REGISTRY_PROP_XFT_LCDFILTER, this->get_xft_rgba() == "rgb" ? "lcddefault" : "none");
        break;
    default:
        break;
    }

    this->registry_.update(XSETTINGS_REGISTRY_PROP_NET_FALLBACK_ICON_THEME, "mate");

    this->xsettings_changed_.emit(key.raw());

    if (is_notify)
    {
        this->registry_.notify();
    }
}

void XSettingsManager::scale_settings()
{
    KLOG_PROFILE("");

    auto scale = this->get_window_scale();
    auto dpi = XSettingsUtils::get_dpi_from_x_server();
    int32_t unscaled_dpi = int32_t(dpi * 1024);
    int32_t scaled_dpi = int32_t(CLAMP(dpi * scale, DPI_LOW_REASONABLE_VALUE, DPI_HIGH_REASONABLE_VALUE) * 1024);
    auto scaled_cursor_size = this->get_gtk_cursor_theme_size() * scale;

    this->registry_.update(XSETTINGS_REGISTRY_PROP_GDK_WINDOW_SCALING_FACTOR, scale);
    this->registry_.update(XSETTINGS_REGISTRY_PROP_GDK_UNSCALED_DPI, unscaled_dpi);
    this->registry_.update(XSETTINGS_REGISTRY_PROP_XFT_DPI, scaled_dpi);
    this->registry_.update(XSETTINGS_REGISTRY_PROP_GTK_CURSOR_THEME_SIZE, scaled_cursor_size);

    this->xsettings_settings_->set_int(XSETTINGS_SCHEMA_XFT_DPI, scaled_dpi);
    this->scale_change_workarounds(scale);
}

void XSettingsManager::scale_change_workarounds(int32_t scale)
{
    KLOG_PROFILE("window_scale: %d, scale: %d", this->window_scale_, scale);

    std::string error;
    RETURN_IF_TRUE(this->window_scale_ == scale);
    // 第一次初始化时设置
    if (!this->window_scale_)
    {
        // 如果开启QT缩放同步，则将缩放值同步到QT缩放相关的环境变量
        if (this->get_window_scaling_factor_qt_sync())
        {
            if (!XSettingsUtils::update_user_env_variable("QT_AUTO_SCREEN_SCALE_FACTOR", "0", error))
            {
                KLOG_WARNING("There was a problem when setting QT_AUTO_SCREEN_SCALE_FACTOR=0: %s", error.c_str());
            }

            if (!XSettingsUtils::update_user_env_variable("QT_SCALE_FACTOR", scale == 2 ? "2" : "1", error))
            {
                KLOG_WARNING("There was a problem when setting QT_SCALE_FACTOR=%d: %s", scale, error.c_str());
            }
        }
    }
    this->window_scale_ = scale;

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
            KLOG_WARNING("There was a problem restarting marco: %s", e.what().c_str());
        }
    }
    // 重启面板
    std::vector<std::string> argv = {"killall", "mate-panel", "kiran-panel"};
    try
    {
        Glib::spawn_async(std::string(), argv, Glib::SPAWN_SEARCH_PATH);
    }
    catch (const Glib::Error &e)
    {
        KLOG_WARNING("There was a problem restarting mate-panel: %s", e.what().c_str());
    }

    // 重置桌面图标大小
    if (this->background_settings_ &&
        this->background_settings_->get_boolean(BACKGROUND_SCHEMA_SHOW_DESKTOP_ICONS) &&
        !this->switch_desktop_icon_[0] &&
        !this->switch_desktop_icon_[1])
    {
        // 延时隐藏/显示桌面图标，给文件管理器一定的时间重绘
        auto timeout = Glib::MainContext::get_default()->signal_timeout();
        this->switch_desktop_icon_[0] = timeout.connect_seconds(sigc::bind(sigc::mem_fun(this, &XSettingsManager::delayed_toggle_bg_draw), false), 1);
        this->switch_desktop_icon_[1] = timeout.connect_seconds(sigc::bind(sigc::mem_fun(this, &XSettingsManager::delayed_toggle_bg_draw), true), 2);
    }
}

void XSettingsManager::on_screen_changed()
{
    KLOG_PROFILE("");

    auto scale = this->get_window_scale();
    if (scale != this->window_scale_)
    {
        this->scale_settings();
    }
    this->registry_.notify();
}

bool XSettingsManager::delayed_toggle_bg_draw(bool value)
{
    KLOG_DEBUG("show-desktop-icons: %d.", value);
    if (this->background_settings_)
    {
        this->background_settings_->set_boolean(BACKGROUND_SCHEMA_SHOW_DESKTOP_ICONS, value);
    }
    return false;
}

void XSettingsManager::on_fontconfig_timestamp_changed()
{
    int32_t timestamp = time(NULL);
    this->registry_.update(XSETTINGS_REGISTRY_PROP_FONTCONFIG_TIMESTAMP, timestamp);
    this->registry_.notify();
}

void XSettingsManager::set_registry_var(std::shared_ptr<XSettingsPropertyBase> var, MethodInvocation &invocation)
{
    auto iter = this->registry2schema_.find(var->get_name());
    if (iter == this->registry2schema_.end())
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_XSETTINGS_PROPERTY_INVALID);
    }

    switch (shash(var->get_name().c_str()))
    {
    case CONNECT(XSETTINGS_REGISTRY_PROP_NET_DOUBLE_CLICK_TIME, _hash):
    case CONNECT(XSETTINGS_REGISTRY_PROP_NET_DOUBLE_CLICK_DISTANCE, _hash):
    case CONNECT(XSETTINGS_REGISTRY_PROP_NET_DND_DRAG_THRESHOLD, _hash):
    case CONNECT(XSETTINGS_REGISTRY_PROP_NET_CURSOR_BLINK_TIME, _hash):
    case CONNECT(XSETTINGS_REGISTRY_PROP_XFT_ANTIALIAS, _hash):
    case CONNECT(XSETTINGS_REGISTRY_PROP_XFT_HINTING, _hash):
    {
        CHECK_VAR(var, XSettingsPropType::XSETTINGS_PROP_TYPE_INT);
        auto int_var = std::dynamic_pointer_cast<XSettingsPropertyInt>(var);
        this->xsettings_settings_->set_int(iter->second, int_var->get_value());
        invocation.ret();
        break;
    }
    case CONNECT(XSETTINGS_REGISTRY_PROP_NET_CURSOR_BLINK, _hash):
    case CONNECT(XSETTINGS_REGISTRY_PROP_NET_ENABLE_EVENT_SOUNDS, _hash):
    case CONNECT(XSETTINGS_REGISTRY_PROP_NET_ENABLE_INPUT_FEEDBACK_SOUNDS, _hash):
    case CONNECT(XSETTINGS_REGISTRY_PROP_GTK_MENU_IMAGES, _hash):
    case CONNECT(XSETTINGS_REGISTRY_PROP_GTK_BUTTON_IMAGES, _hash):
    case CONNECT(XSETTINGS_REGISTRY_PROP_GTK_SHELL_SHOWS_APP_MENU, _hash):
    case CONNECT(XSETTINGS_REGISTRY_PROP_GTK_SHELL_SHOWS_MENUBAR, _hash):
    case CONNECT(XSETTINGS_REGISTRY_PROP_GTK_SHOW_INPUT_METHOD_MENU, _hash):
    case CONNECT(XSETTINGS_REGISTRY_PROP_GTK_SHOW_UNICODE_MENU, _hash):
    case CONNECT(XSETTINGS_REGISTRY_PROP_GTK_AUTO_MNEMONICS, _hash):
    case CONNECT(XSETTINGS_REGISTRY_PROP_GTK_ENABLE_PRIMARY_PASTE, _hash):
    case CONNECT(XSETTINGS_REGISTRY_PROP_GTK_ENABLE_ANIMATIONS, _hash):
    case CONNECT(XSETTINGS_REGISTRY_PROP_GTK_DIALOGS_USE_HEADER, _hash):
    {
        CHECK_VAR(var, XSettingsPropType::XSETTINGS_PROP_TYPE_INT);
        auto int_var = std::dynamic_pointer_cast<XSettingsPropertyInt>(var);
        this->xsettings_settings_->set_boolean(iter->second, int_var->get_value());
        invocation.ret();
        break;
    }
    case CONNECT(XSETTINGS_REGISTRY_PROP_NET_THEME_NAME, _hash):
    case CONNECT(XSETTINGS_REGISTRY_PROP_NET_ICON_THEME_NAME, _hash):
    case CONNECT(XSETTINGS_REGISTRY_PROP_NET_SOUND_THEME_NAME, _hash):
    case CONNECT(XSETTINGS_REGISTRY_PROP_XFT_HINT_STYLE, _hash):
    case CONNECT(XSETTINGS_REGISTRY_PROP_XFT_RGBA, _hash):
    case CONNECT(XSETTINGS_REGISTRY_PROP_GTK_CURSOR_THEME_NAME, _hash):
    case CONNECT(XSETTINGS_REGISTRY_PROP_GTK_FONT_NAME, _hash):
    case CONNECT(XSETTINGS_REGISTRY_PROP_GTK_KEY_THEME_NAME, _hash):
    case CONNECT(XSETTINGS_REGISTRY_PROP_GTK_TOOLBAR_STYLE, _hash):
    case CONNECT(XSETTINGS_REGISTRY_PROP_GTK_TOOLBAR_ICON_SIZE, _hash):
    case CONNECT(XSETTINGS_REGISTRY_PROP_GTK_IM_PREEDIT_STYLE, _hash):
    case CONNECT(XSETTINGS_REGISTRY_PROP_GTK_IM_STATUS_STYLE, _hash):
    case CONNECT(XSETTINGS_REGISTRY_PROP_GTK_IM_MODULE, _hash):
    case CONNECT(XSETTINGS_REGISTRY_PROP_GTK_MENU_BAR_ACCEL, _hash):
    case CONNECT(XSETTINGS_REGISTRY_PROP_GTK_COLOR_SCHEME, _hash):
    case CONNECT(XSETTINGS_REGISTRY_PROP_GTK_FILE_CHOOSER_BACKEND, _hash):
    case CONNECT(XSETTINGS_REGISTRY_PROP_GTK_DECORATION_LAYOUT, _hash):
    {
        CHECK_VAR(var, XSettingsPropType::XSETTINGS_PROP_TYPE_STRING);
        auto string_var = std::dynamic_pointer_cast<XSettingsPropertyString>(var);
        this->xsettings_settings_->set_string(iter->second, string_var->get_value());
        invocation.ret();
        break;
    }
    case CONNECT(XSETTINGS_REGISTRY_PROP_XFT_LCDFILTER, _hash):
    case CONNECT(XSETTINGS_REGISTRY_PROP_NET_FALLBACK_ICON_THEME, _hash):
    case CONNECT(XSETTINGS_REGISTRY_PROP_GDK_WINDOW_SCALING_FACTOR, _hash):
    case CONNECT(XSETTINGS_REGISTRY_PROP_GDK_UNSCALED_DPI, _hash):
    case CONNECT(XSETTINGS_REGISTRY_PROP_XFT_DPI, _hash):
    case CONNECT(XSETTINGS_REGISTRY_PROP_GTK_CURSOR_THEME_SIZE, _hash):
    case CONNECT(XSETTINGS_REGISTRY_PROP_FONTCONFIG_TIMESTAMP, _hash):
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_XSETTINGS_PROPERTY_ONLYREAD);
        break;
    default:
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_XSETTINGS_PROPERTY_UNSUPPORTED);
        break;
    }
}

void XSettingsManager::on_bus_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name)
{
    KLOG_PROFILE("name: %s", name.c_str());
    if (!connect)
    {
        KLOG_WARNING("failed to connect dbus. name: %s", name.c_str());
        return;
    }
    try
    {
        this->object_register_id_ = this->register_object(connect, XSETTINGS_OBJECT_PATH);
    }
    catch (const Glib::Error &e)
    {
        KLOG_WARNING("register object_path %s fail: %s.", XSETTINGS_OBJECT_PATH, e.what().c_str());
    }
}

void XSettingsManager::on_name_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name)
{
    KLOG_DEBUG("success to register dbus name: %s", name.c_str());
}

void XSettingsManager::on_name_lost(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name)
{
    KLOG_WARNING("failed to register dbus name: %s", name.c_str());
}

}  // namespace Kiran