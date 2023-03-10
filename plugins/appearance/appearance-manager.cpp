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

#include "plugins/appearance/appearance-manager.h"

#include <json/json.h>

namespace Kiran
{
AppearanceManager::AppearanceManager() : auto_switch_window_theme_(false),
                                         dbus_connect_id_(0),
                                         object_register_id_(0)

{
    this->appearance_settings_ = Gio::Settings::create(APPEARANCE_SCHAME_ID);
}

AppearanceManager::~AppearanceManager()
{
    if (this->dbus_connect_id_)
    {
        Gio::DBus::unown_name(this->dbus_connect_id_);
    }
}

AppearanceManager* AppearanceManager::instance_ = nullptr;
void AppearanceManager::global_init()
{
    instance_ = new AppearanceManager();
    instance_->init();
}

void AppearanceManager::GetThemes(gint32 type, MethodInvocation& invocation)
{
    if (type < 0 || type >= int32_t(AppearanceThemeType::APPEARANCE_THEME_TYPE_LAST))
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_APPEARANCE_THEME_TYPE_INVALID);
    }

    auto themes = this->appearance_theme_.get_themes_by_type(AppearanceThemeType(type));
    Json::Value json_themes;
    Json::FastWriter writer;
    for (uint32_t i = 0; i < themes.size(); ++i)
    {
        json_themes[i]["name"] = themes[i]->name;
        json_themes[i]["path"] = themes[i]->path;
    }
    auto result = writer.write(json_themes);
    invocation.ret(result);
}

void AppearanceManager::SetTheme(gint32 type, const Glib::ustring& theme_name, MethodInvocation& invocation)
{
    KLOG_DEBUG("Set theme which type is (%d) to %s", type, theme_name.c_str());

    ThemeKey key = std::make_pair(type, theme_name);
    CCErrorCode error_code = CCErrorCode::SUCCESS;
    if (!this->appearance_theme_.set_theme(key, error_code))
    {
        DBUS_ERROR_REPLY_AND_RET(error_code);
    }

    // 如果手动设置了GTK或者窗口标题主题，则取消主题自动切换
    if (type == AppearanceThemeType::APPEARANCE_THEME_TYPE_GTK ||
        type == AppearanceThemeType::APPEARANCE_THEME_TYPE_METACITY)
    {
        this->AutoSwitchWindowTheme_set(false);
    }

    invocation.ret();
}

void AppearanceManager::EnableAutoSwitchWindowTheme(MethodInvocation& invocation)
{
    this->AutoSwitchWindowTheme_set(true);
    invocation.ret();
}

void AppearanceManager::GetTheme(gint32 type, MethodInvocation& invocation)
{
    invocation.ret(this->appearance_theme_.get_theme(AppearanceThemeType(type)));
}

void AppearanceManager::GetFont(gint32 type, MethodInvocation& invocation)
{
    if (type < 0 || type >= int32_t(AppearanceFontType::APPEARANCE_FONT_TYPE_LAST))
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_APPEARANCE_FONT_TYPE_INVALID_1);
    }
    invocation.ret(this->appearance_font_.get_font(AppearanceFontType(type)));
}

void AppearanceManager::SetFont(gint32 type, const Glib::ustring& font, MethodInvocation& invocation)
{
    KLOG_DEBUG("Set font which type is (%d) to %s", type, font.c_str());

    if (type < 0 || type >= int32_t(AppearanceFontType::APPEARANCE_FONT_TYPE_LAST))
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_APPEARANCE_FONT_TYPE_INVALID_2);
    }

    if (!this->appearance_font_.set_font(AppearanceFontType(type), font))
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_APPEARANCE_FONT_TYPE_UNSUPPORTED);
    }
    invocation.ret();
}

void AppearanceManager::SetDesktopBackground(const Glib::ustring& desktop_background, MethodInvocation& invocation)
{
    KLOG_DEBUG("Set desktop background to %s", desktop_background.c_str());

    if (desktop_background != this->desktop_background_get() &&
        !this->desktop_background_set(desktop_background))
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_APPEARANCE_SET_BACKGROUND_FAILED);
    }
    invocation.ret();
}

void AppearanceManager::SetLockScreenBackground(const Glib::ustring& lock_screen_background, MethodInvocation& invocation)
{
    KLOG_DEBUG("Set lock screen background to %s", lock_screen_background.c_str());

    if (lock_screen_background != this->lock_screen_background_get() &&
        !this->lock_screen_background_set(lock_screen_background))
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_APPEARANCE_SET_LOCKSCREEN_BACKGROUND_FAILED);
    }
    invocation.ret();
}

bool AppearanceManager::desktop_background_setHandler(const Glib::ustring& value)
{
    RETURN_VAL_IF_TRUE(value == this->desktop_background_, false);
    this->desktop_background_ = value;

    if (this->appearance_settings_->get_string(APPEARANCE_SCHEMA_KEY_DESKTOP_BG) != value)
    {
        this->appearance_settings_->set_string(APPEARANCE_SCHEMA_KEY_DESKTOP_BG, value);
        return true;
    }

    return false;
}

bool AppearanceManager::lock_screen_background_setHandler(const Glib::ustring& value)
{
    RETURN_VAL_IF_TRUE(value == this->lock_screen_background_, false);
    this->lock_screen_background_ = value;

    if (this->appearance_settings_->get_string(APPEARANCE_SCHEMA_KEY_LOCKSCREEN_BG) != value)
    {
        this->appearance_settings_->set_string(APPEARANCE_SCHEMA_KEY_LOCKSCREEN_BG, value);
        return true;
    }
    return false;
}

bool AppearanceManager::AutoSwitchWindowTheme_setHandler(bool value)
{
    RETURN_VAL_IF_TRUE(value == this->auto_switch_window_theme_, false);
    this->auto_switch_window_theme_ = value;

    if (this->appearance_settings_->get_boolean(APPEARANCE_SCHEMA_KEY_AUTO_SWITCH_WINDOW_THEME) != value)
    {
        this->appearance_settings_->set_boolean(APPEARANCE_SCHEMA_KEY_AUTO_SWITCH_WINDOW_THEME, value);
        return true;
    }
    return false;
}

void AppearanceManager::init()
{
    this->appearance_theme_.init();
    this->appearance_font_.init();
    this->appearance_background_.init();

    this->load_from_settings();

    this->appearance_theme_.signal_theme_changed().connect(sigc::mem_fun(this, &AppearanceManager::on_theme_changed_cb));
    this->appearance_font_.signal_font_changed().connect(sigc::mem_fun(this, &AppearanceManager::on_font_chnaged_cb));
    this->appearance_settings_->signal_changed().connect(sigc::mem_fun(this, &AppearanceManager::on_settings_changed_cb));

    this->dbus_connect_id_ = Gio::DBus::own_name(Gio::DBus::BUS_TYPE_SESSION,
                                                 APPEARANCE_DBUS_NAME,
                                                 sigc::mem_fun(this, &AppearanceManager::on_bus_acquired),
                                                 sigc::mem_fun(this, &AppearanceManager::on_name_acquired),
                                                 sigc::mem_fun(this, &AppearanceManager::on_name_lost));
}

void AppearanceManager::load_from_settings()
{
    for (const auto& key : this->appearance_settings_->list_keys())
    {
        this->on_settings_changed_cb(key);
    }
}

void AppearanceManager::auto_switch_for_window_theme()
{
    auto current_datetime = Glib::DateTime::create_now_local();
    auto current_hour = current_datetime.get_hour();
    auto error_code = CCErrorCode::SUCCESS;

    // 下午8点之后到早上8点之前判定为晚上，使用深色主题，否则使用浅色主题
    auto theme_name = (current_hour < 8 || current_hour > 20) ? APPEARANCE_DEFAULT_DARK_GTK_THEME : APPEARANCE_DEFAULT_LIGHT_GTK_THEME;
    if (!this->appearance_theme_.set_theme(std::make_pair(AppearanceThemeType::APPEARANCE_THEME_TYPE_GTK, theme_name),
                                           error_code))
    {
        KLOG_WARNING("Failed to set window gtk theme: %x.", error_code);
    }

    if (!this->appearance_theme_.set_theme(std::make_pair(AppearanceThemeType::APPEARANCE_THEME_TYPE_METACITY, theme_name),
                                           error_code))
    {
        KLOG_WARNING("Failed to set window metacity theme: %x.", error_code);
    }
}

void AppearanceManager::on_theme_changed_cb(ThemeKey theme_key)
{
    KLOG_PROFILE("type: %d, theme name: %s.", theme_key.first, theme_key.second.c_str());

    this->ThemeChanged_signal.emit(theme_key.first, theme_key.second);
}

void AppearanceManager::on_font_chnaged_cb(AppearanceFontType type, const std::string& font)
{
    KLOG_PROFILE("type: %d, font: %s.", int32_t(type), font.c_str());

    this->FontChanged_signal.emit(int32_t(type), font);
}

void AppearanceManager::on_settings_changed_cb(const Glib::ustring& key)
{
    KLOG_PROFILE("key: %s", key.c_str());

    switch (shash(key.c_str()))
    {
    case CONNECT(APPEARANCE_SCHEMA_KEY_DESKTOP_BG, _hash):
    {
        this->desktop_background_set(this->appearance_settings_->get_string(APPEARANCE_SCHEMA_KEY_DESKTOP_BG));
        break;
    }
    case CONNECT(APPEARANCE_SCHEMA_KEY_LOCKSCREEN_BG, _hash):
    {
        this->lock_screen_background_set(this->appearance_settings_->get_string(APPEARANCE_SCHEMA_KEY_LOCKSCREEN_BG));
        break;
    }
    case CONNECT(APPEARANCE_SCHEMA_KEY_AUTO_SWITCH_WINDOW_THEME, _hash):
    {
        this->AutoSwitchWindowTheme_set(this->appearance_settings_->get_boolean(APPEARANCE_SCHEMA_KEY_AUTO_SWITCH_WINDOW_THEME));
        if (this->AutoSwitchWindowTheme_get())
        {
            this->auto_switch_for_window_theme();
        }
        break;
    }
    default:
        break;
    }
}

void AppearanceManager::on_bus_acquired(const Glib::RefPtr<Gio::DBus::Connection>& connect, Glib::ustring name)
{
    KLOG_PROFILE("name: %s", name.c_str());
    if (!connect)
    {
        KLOG_WARNING("Failed to connect dbus. name: %s", name.c_str());
        return;
    }
    try
    {
        this->object_register_id_ = this->register_object(connect, APPEARANCE_OBJECT_PATH);
    }
    catch (const Glib::Error& e)
    {
        KLOG_WARNING("Register object_path %s fail: %s.", APPEARANCE_OBJECT_PATH, e.what().c_str());
    }
}

void AppearanceManager::on_name_acquired(const Glib::RefPtr<Gio::DBus::Connection>& connect, Glib::ustring name)
{
    KLOG_DEBUG("Success to register dbus name: %s", name.c_str());
}

void AppearanceManager::on_name_lost(const Glib::RefPtr<Gio::DBus::Connection>& connect, Glib::ustring name)
{
    KLOG_WARNING("Failed to register dbus name: %s", name.c_str());
}
}  // namespace  Kiran
