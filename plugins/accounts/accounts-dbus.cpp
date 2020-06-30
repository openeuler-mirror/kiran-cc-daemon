/*
 * @Author       : tangjie02
 * @Date         : 2020-06-19 13:58:22
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-06-30 15:08:11
 * @Description  : 
 * @FilePath     : /kiran-system-daemon/plugins/accounts/accounts-dbus.cpp
 */

#include "plugins/accounts/accounts-dbus.h"

namespace Kiran
{
#define MOUSE_DBUS_NAME "com.unikylin.Kiran.DBus.Mouse"

#define MOUSE_DBUS_OBJECT "com.unikylin.Kiran.DBus.Mouse"

AccountsDBus::AccountsDBus() : dbus_connect_id_(0),
                               object_register_id_(0)
{
}

AccountsDBus::~AccountsDBus()
{
    if (this->dbus_connect_id_)
    {
        Gio::DBus::unown_name(this->dbus_connect_id_);
    }
    if (this->object_register_id_)
    {
        this->unregister_object();
    }
}

void AccountsDBus::init()
{
    this->dbus_connect_id_ = Gio::DBus::own_name(Gio::DBus::BUS_TYPE_SESSION,
                                                 MOUSE_DBUS_NAME,
                                                 sigc::mem_fun(this, &AccountsDBus::on_bus_acquired),
                                                 sigc::mem_fun(this, &AccountsDBus::on_name_acquired),
                                                 sigc::mem_fun(this, &AccountsDBus::on_name_lost));
}

void AccountsDBus::Reset(MethodInvocation& invocation)
{
}

void AccountsDBus::on_bus_acquired(const Glib::RefPtr<Gio::DBus::Connection>& connect, Glib::ustring name)
{
    this->object_register_id_ = this->register_object(connect, MOUSE_DBUS_OBJECT);
}

void AccountsDBus::on_name_acquired(const Glib::RefPtr<Gio::DBus::Connection>& connect, Glib::ustring name)
{
}

void AccountsDBus::on_name_lost(const Glib::RefPtr<Gio::DBus::Connection>& connect, Glib::ustring name)
{
}
}  // namespace Kiran