/*
 * @Author       : tangjie02
 * @Date         : 2020-12-01 10:15:50
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-12-09 16:33:51
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/plugins/appearance/appearance-manager.cpp
 */
#include "plugins/appearance/appearance-manager.h"

#include <glib/gi18n.h>
#include <json/json.h>

namespace Kiran
{
AppearanceManager::AppearanceManager() : dbus_connect_id_(0),
                                         object_register_id_(0)
{
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
        DBUS_ERROR_REPLY_AND_RET(CCError::ERROR_FAILED, _("Invalid type"));
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
    ThemeKey key = std::make_pair(type, theme_name);
    std::string error;
    if (!this->appearance_theme_.set_theme(key, error))
    {
        DBUS_ERROR_REPLY_AND_RET(CCError::ERROR_FAILED, error.c_str());
    }
    invocation.ret();
}

void AppearanceManager::GetFont(gint32 type, MethodInvocation& invocation)
{
    if (type < 0 || type >= int32_t(AppearanceFontType::APPEARANCE_FONT_TYPE_LAST))
    {
        DBUS_ERROR_REPLY_AND_RET(CCError::ERROR_FAILED, _("Invalid type"));
    }
    invocation.ret(this->appearance_font_.get_font(AppearanceFontType(type)));
}

void AppearanceManager::SetFont(gint32 type, const Glib::ustring& font, MethodInvocation& invocation)
{
    if (type < 0 || type >= int32_t(AppearanceFontType::APPEARANCE_FONT_TYPE_LAST))
    {
        DBUS_ERROR_REPLY_AND_RET(CCError::ERROR_FAILED, _("Invalid type"));
    }

    if (!this->appearance_font_.set_font(AppearanceFontType(type), font))
    {
        DBUS_ERROR_REPLY_AND_RET(CCError::ERROR_FAILED, _("Uunsupported type"));
    }
    invocation.ret();
}

void AppearanceManager::init()
{
    this->appearance_theme_.init();
    this->appearance_font_.init();

    this->dbus_connect_id_ = Gio::DBus::own_name(Gio::DBus::BUS_TYPE_SESSION,
                                                 APPEARANCE_DBUS_NAME,
                                                 sigc::mem_fun(this, &AppearanceManager::on_bus_acquired),
                                                 sigc::mem_fun(this, &AppearanceManager::on_name_acquired),
                                                 sigc::mem_fun(this, &AppearanceManager::on_name_lost));
}

void AppearanceManager::on_bus_acquired(const Glib::RefPtr<Gio::DBus::Connection>& connect, Glib::ustring name)
{
    SETTINGS_PROFILE("name: %s", name.c_str());
    if (!connect)
    {
        LOG_WARNING("Failed to connect dbus. name: %s", name.c_str());
        return;
    }
    try
    {
        this->object_register_id_ = this->register_object(connect, APPEARANCE_OBJECT_PATH);
    }
    catch (const Glib::Error& e)
    {
        LOG_WARNING("Register object_path %s fail: %s.", APPEARANCE_OBJECT_PATH, e.what().c_str());
    }
}

void AppearanceManager::on_name_acquired(const Glib::RefPtr<Gio::DBus::Connection>& connect, Glib::ustring name)
{
    LOG_DEBUG("Success to register dbus name: %s", name.c_str());
}

void AppearanceManager::on_name_lost(const Glib::RefPtr<Gio::DBus::Connection>& connect, Glib::ustring name)
{
    LOG_WARNING("Failed to register dbus name: %s", name.c_str());
}
}  // namespace  Kiran
