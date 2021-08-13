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

#include "plugins/power/wrapper/power-login1.h"

namespace Kiran
{
#define LOGIN1_DBUS_NAME "org.freedesktop.login1"
#define LOGIN1_DBUS_OBJECT_PATH "/org/freedesktop/login1"
#define LOGIN1_MANAGER_DBUS_INTERFACE "org.freedesktop.login1.Manager"

PowerLogin1::PowerLogin1()
{
}

void PowerLogin1::init()
{
    try
    {
        this->login1_proxy_ = Gio::DBus::Proxy::create_for_bus_sync(Gio::DBus::BUS_TYPE_SYSTEM,
                                                                    LOGIN1_DBUS_NAME,
                                                                    LOGIN1_DBUS_OBJECT_PATH,
                                                                    LOGIN1_MANAGER_DBUS_INTERFACE);
    }
    catch (const Glib::Error& e)
    {
        KLOG_WARNING("%s", e.what().c_str());
        return;
    }
}

bool PowerLogin1::suspend()
{
    KLOG_PROFILE("");

    RETURN_VAL_IF_FALSE(this->login1_proxy_, false);

    auto parameters = g_variant_new("(b)", FALSE);
    Glib::VariantContainerBase base(parameters, false);

    try
    {
        this->login1_proxy_->call_sync("Suspend", base);
    }
    catch (const Glib::Error& e)
    {
        KLOG_WARNING("%s", e.what().c_str());
        return false;
    }
    return true;
}

bool PowerLogin1::hibernate()
{
    KLOG_PROFILE("");

    RETURN_VAL_IF_FALSE(this->login1_proxy_, false);

    auto parameters = g_variant_new("(b)", FALSE);
    Glib::VariantContainerBase base(parameters, false);

    try
    {
        this->login1_proxy_->call_sync("Hibernate", base);
    }
    catch (const Glib::Error& e)
    {
        KLOG_WARNING("%s", e.what().c_str());
        return false;
    }
    return true;
}

bool PowerLogin1::shutdown()
{
    KLOG_PROFILE("");

    RETURN_VAL_IF_FALSE(this->login1_proxy_, false);

    auto parameters = g_variant_new("(b)", FALSE);
    Glib::VariantContainerBase base(parameters, false);

    try
    {
        this->login1_proxy_->call_sync("PowerOff", base);
    }
    catch (const Glib::Error& e)
    {
        KLOG_WARNING("%s", e.what().c_str());
        return false;
    }
    return true;
}

}  // namespace Kiran