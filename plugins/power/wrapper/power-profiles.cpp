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

#include "plugins/power/wrapper/power-profiles.h"

namespace Kiran
{
#define PROFILES_DBUS_NAME "net.hadess.PowerProfiles"
#define PROFILES_DBUS_OBJECT_PATH "/net/hadess/PowerProfiles"
#define PROFILES_DBUS_INTERFACE "net.hadess.PowerProfiles"
#define PROFILES_DBUS_PROP_ACTIVE_PROFILE "ActiveProfile"

PowerProfiles::PowerProfiles()
{
    try
    {
        this->profiles_proxy_ = Gio::DBus::Proxy::create_for_bus_sync(Gio::DBus::BUS_TYPE_SYSTEM,
                                                                      PROFILES_DBUS_NAME,
                                                                      PROFILES_DBUS_OBJECT_PATH,
                                                                      PROFILES_DBUS_INTERFACE);
    }
    catch (const Glib::Error &e)
    {
        KLOG_WARNING("Failed to create bus sync: %s", e.what().c_str());
        return;
    }
}

void PowerProfiles::init()
{
    this->profiles_proxy_->signal_properties_changed().connect(sigc::mem_fun(this, &PowerProfiles::on_properties_changed));
}

uint32_t PowerProfiles::hold_profile(const std::string &profile,
                                     const std::string &reason,
                                     const std::string &application_id)
{
    Glib::VariantContainerBase retval;

    RETURN_VAL_IF_FALSE(this->profiles_proxy_, false);

    auto parameters = g_variant_new("(sss)", profile.c_str(), reason.c_str(), application_id.c_str());
    Glib::VariantContainerBase base(parameters, false);

    KLOG_DEBUG("Set power active profile to %s.", profile.c_str());

    try
    {
        retval = this->profiles_proxy_->call_sync("HoldProfile", base);
    }
    catch (const Glib::Error &e)
    {
        KLOG_WARNING("%s", e.what().c_str());
        return 0;
    }

    try
    {
        auto v1 = retval.get_child(0);
        auto cookie = Glib::VariantBase::cast_dynamic<Glib::Variant<uint32_t>>(v1).get();
        return cookie;
    }
    catch (const std::exception &e)
    {
        KLOG_WARNING("%s", e.what());
    }

    return 0;
}

void PowerProfiles::release_profile(uint32_t cookie)
{
    Glib::VariantContainerBase retval;

    RETURN_IF_FALSE(this->profiles_proxy_);

    auto parameters = g_variant_new("(u)", cookie);
    Glib::VariantContainerBase base(parameters, false);

    try
    {
        this->profiles_proxy_->call_sync("ReleaseProfile", base);
    }
    catch (const Glib::Error &e)
    {
        KLOG_WARNING("%s", e.what().c_str());
    }
}

std::string PowerProfiles::get_active_profile()
{
    RETURN_VAL_IF_FALSE(this->profiles_proxy_, POWER_PROFILE_BALANCED);

    try
    {
        Glib::VariantBase value;
        this->profiles_proxy_->get_cached_property(value, PROFILES_DBUS_PROP_ACTIVE_PROFILE);
        return Glib::VariantBase::cast_dynamic<Glib::Variant<Glib::ustring>>(value).get();
    }
    catch (const std::exception &e)
    {
        KLOG_WARNING("%s", e.what());
    }
    // 默认返回平衡模式
    return POWER_PROFILE_BALANCED;
}

void PowerProfiles::on_properties_changed(const Gio::DBus::Proxy::MapChangedProperties &changed_properties,
                                          const std::vector<Glib::ustring> &invalidated_properties)
{
    try
    {
        for (auto &iter : changed_properties)
        {
            switch (shash(iter.first.c_str()))
            {
            case CONNECT(PROFILES_DBUS_PROP_ACTIVE_PROFILE, _hash):
            {
                auto active_profile = Glib::VariantBase::cast_dynamic<Glib::Variant<Glib::ustring>>(iter.second).get();
                this->active_profile_changed_.emit(active_profile);
                break;
            }
            default:
                break;
            }
        }
    }
    catch (const std::exception &e)
    {
        KLOG_WARNING("%s", e.what());
    }

    return;
}

}  // namespace Kiran
