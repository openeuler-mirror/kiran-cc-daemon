/*
 * @Author       : tangjie02
 * @Date         : 2020-08-11 16:21:11
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-08-11 17:00:00
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/plugins/xsettings/xsettings-manager.cpp
 */

#include "plugins/xsettings/xsettings-manager.h"

#include "lib/base/log.h"

namespace Kiran
{
#define GSETTINGS_DBUS_NAME "com.unikylin.Kiran.SessionDaemon.XSettings"
#define GSETTINGS_OBJECT_PATH "/com/unikylin/Kiran/SessionDaemon/XSettings"

#define MOUSE_SCHEMA_ID "org.mate.peripherals-mouse"
#define MOUSE_SCHEMA_DOUBLE_CLICK "double-click"

XSettingsManager::XSettingsManager() : dbus_connect_id_(0),
                                       object_register_id_(0),
                                       double_click_time_(250)
{
    this->mouse_settings_ = Gio::Settings::create(MOUSE_SCHEMA_ID);
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

bool XSettingsManager::double_click_time_setHandler(gint32 value)
{
    SETTINGS_PROFILE("");
    RETURN_VAL_IF_TRUE(value == this->double_click_time_, false);

    this->double_click_time_ = value;
    this->mouse_settings_->set_int(MOUSE_SCHEMA_DOUBLE_CLICK, value);

    return true;
}

void XSettingsManager::init()
{
    this->load_from_settings();

    this->mouse_settings_->signal_changed().connect(sigc::mem_fun(this, &XSettingsManager::settings_changed));

    this->dbus_connect_id_ = Gio::DBus::own_name(Gio::DBus::BUS_TYPE_SESSION,
                                                 GSETTINGS_DBUS_NAME,
                                                 sigc::mem_fun(this, &XSettingsManager::on_bus_acquired),
                                                 sigc::mem_fun(this, &XSettingsManager::on_name_acquired),
                                                 sigc::mem_fun(this, &XSettingsManager::on_name_lost));
}

void XSettingsManager::load_from_settings()
{
    SETTINGS_PROFILE("");

    if (this->mouse_settings_)
    {
        this->double_click_time_ = this->mouse_settings_->get_int(MOUSE_SCHEMA_DOUBLE_CLICK);
    }
}

void XSettingsManager::settings_changed(const Glib::ustring &key)
{
    SETTINGS_PROFILE("key: %s.", key.c_str());

    switch (shash(key.c_str()))
    {
    case "double-click"_hash:
        this->double_click_time_set(this->mouse_settings_->get_int(MOUSE_SCHEMA_DOUBLE_CLICK));
        break;
    default:
        break;
    }
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