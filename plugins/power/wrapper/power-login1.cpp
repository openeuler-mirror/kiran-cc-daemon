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