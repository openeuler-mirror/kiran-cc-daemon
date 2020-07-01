/*
 * @Author       : tangjie02
 * @Date         : 2020-06-19 13:58:22
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-07-01 14:37:27
 * @Description  : 
 * @FilePath     : /kiran-system-daemon/plugins/accounts/user.cpp
 */

#include "plugins/accounts/user.h"

namespace Kiran
{
#define MOUSE_DBUS_NAME "com.unikylin.Kiran.DBus.Mouse"

#define MOUSE_DBUS_OBJECT "com.unikylin.Kiran.DBus.Mouse"

User::User(ActUser* act_user) : dbus_connect_id_(0),
                                object_register_id_(0),
                                act_user_(act_user)
{
}

User::~User()
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

void User::init()
{
    this->dbus_connect_id_ = Gio::DBus::own_name(Gio::DBus::BUS_TYPE_SESSION,
                                                 MOUSE_DBUS_NAME,
                                                 sigc::mem_fun(this, &User::on_bus_acquired),
                                                 sigc::mem_fun(this, &User::on_name_acquired),
                                                 sigc::mem_fun(this, &User::on_name_lost));
}

void User::Reset(MethodInvocation& invocation)
{
}

void User::on_bus_acquired(const Glib::RefPtr<Gio::DBus::Connection>& connect, Glib::ustring name)
{
    this->object_register_id_ = this->register_object(connect, MOUSE_DBUS_OBJECT);
}

void User::on_name_acquired(const Glib::RefPtr<Gio::DBus::Connection>& connect, Glib::ustring name)
{
}

void User::on_name_lost(const Glib::RefPtr<Gio::DBus::Connection>& connect, Glib::ustring name)
{
}
}  // namespace Kiran