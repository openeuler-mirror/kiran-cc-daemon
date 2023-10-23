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
        KLOG_WARNING_POWER("%s", e.what().c_str());
        return;
    }
}

int32_t PowerLogin1::inhibit(const std::string& what)
{
    try
    {
        Glib::RefPtr<Gio::UnixFDList> fd_list;
        Glib::RefPtr<Gio::UnixFDList> out_fd_list;
        auto g_parameters = g_variant_new("(ssss)", what.c_str(),
                                          Glib::get_user_name().c_str(),
                                          "The power plugin of kiran-session-daemon handles these events",
                                          "block");
        Glib::VariantContainerBase parameters(g_parameters, false);
        auto retval = this->login1_proxy_->call_sync("Inhibit", parameters, fd_list, out_fd_list);
        auto v1 = retval.get_child(0);
        auto fd_index = Glib::VariantBase::cast_dynamic<Glib::Variant<int32_t>>(v1).get();
        auto fd = out_fd_list->get(fd_index);
        KLOG_DEBUG_POWER("Inhibit file descriptor[index: %d]: %d.", fd_index, fd);
        return fd;
    }
    catch (const Glib::Error& e)
    {
        KLOG_WARNING_POWER("%s", e.what().c_str());
    }
    catch (const std::exception& e)
    {
        KLOG_WARNING_POWER("%s", e.what());
    }
    return -1;
}

bool PowerLogin1::suspend()
{
    RETURN_VAL_IF_FALSE(this->login1_proxy_, false);

    auto g_parameters = g_variant_new("(b)", FALSE);
    Glib::VariantContainerBase parameters(g_parameters, false);

    try
    {
        this->login1_proxy_->call_sync("Suspend", parameters);
    }
    catch (const Glib::Error& e)
    {
        KLOG_WARNING_POWER("%s", e.what().c_str());
        return false;
    }
    return true;
}

bool PowerLogin1::hibernate()
{
    RETURN_VAL_IF_FALSE(this->login1_proxy_, false);

    auto g_parameters = g_variant_new("(b)", FALSE);
    Glib::VariantContainerBase parameters(g_parameters, false);

    try
    {
        this->login1_proxy_->call_sync("Hibernate", parameters);
    }
    catch (const Glib::Error& e)
    {
        KLOG_WARNING_POWER("%s", e.what().c_str());
        return false;
    }
    return true;
}

bool PowerLogin1::shutdown()
{
    RETURN_VAL_IF_FALSE(this->login1_proxy_, false);

    auto g_parameters = g_variant_new("(b)", FALSE);
    Glib::VariantContainerBase parameters(g_parameters, false);

    try
    {
        this->login1_proxy_->call_sync("PowerOff", parameters);
    }
    catch (const Glib::Error& e)
    {
        KLOG_WARNING_POWER("%s", e.what().c_str());
        return false;
    }
    return true;
}

}  // namespace Kiran