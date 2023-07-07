/**
 * Copyright (c) 2023 ~ 2024 KylinSec Co., Ltd. 
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

#include "plugins/power/wrapper/power-profiles-hadess.h"
#include "power-i.h"

namespace Kiran
{
#define PROFILES_HADESS_MODE_SAVER "power-saver"
#define PROFILES_HADESS_MODE_BALANCED "balanced"
#define PROFILES_HADESS_MODE_PERFORMANCE "performance"

#define PROFILES_HADESS_DBUS_NAME "net.hadess.PowerProfiles"
#define PROFILES_HADESS_DBUS_OBJECT_PATH "/net/hadess/PowerProfiles"
#define PROFILES_HADESS_DBUS_INTERFACE "net.hadess.PowerProfiles"
#define PROFILES_HADESS_DBUS_PROP_ACTIVE_PROFILE "ActiveProfile"

PowerProfilesHadess::PowerProfilesHadess()
{
    try
    {
        this->profiles_proxy_ = Gio::DBus::Proxy::create_for_bus_sync(Gio::DBus::BUS_TYPE_SYSTEM,
                                                                      PROFILES_HADESS_DBUS_NAME,
                                                                      PROFILES_HADESS_DBUS_OBJECT_PATH,
                                                                      PROFILES_HADESS_DBUS_INTERFACE);
    }
    catch (const Glib::Error &e)
    {
        KLOG_WARNING("Failed to create bus sync: %s", e.what().c_str());
        return;
    }
}

void PowerProfilesHadess::init()
{
    this->profiles_proxy_->signal_properties_changed().connect(sigc::mem_fun(this, &PowerProfilesHadess::on_properties_changed));
}

bool PowerProfilesHadess::switch_profile(int32_t profile_mode)
{
    RETURN_VAL_IF_FALSE(this->profiles_proxy_, false);

    auto profile_mode_str = this->porfile_mode_enum2str(profile_mode);
    KLOG_DEBUG("Switch power active profile to %s.", profile_mode_str.c_str());

    try
    {
        std::vector<Glib::VariantBase> params_base;
        params_base.push_back(Glib::Variant<Glib::ustring>::create(PROFILES_HADESS_DBUS_INTERFACE));
        params_base.push_back(Glib::Variant<Glib::ustring>::create(PROFILES_HADESS_DBUS_PROP_ACTIVE_PROFILE));
        params_base.push_back(Glib::Variant<Glib::VariantBase>::create(Glib::Variant<Glib::ustring>::create((profile_mode_str))));
        Glib::VariantContainerBase params = Glib::VariantContainerBase::create_tuple(params_base);
        this->profiles_proxy_->call_sync("org.freedesktop.DBus.Properties.Set", params);
    }
    catch (const Glib::Error &e)
    {
        KLOG_WARNING("Failed to set property %s to %s: %s",
                     PROFILES_HADESS_DBUS_PROP_ACTIVE_PROFILE,
                     profile_mode_str.c_str(),
                     e.what().c_str());
        return false;
    }
    catch (const std::exception &e)
    {
        KLOG_WARNING("Failed to set property %s to %s: %s",
                     PROFILES_HADESS_DBUS_PROP_ACTIVE_PROFILE,
                     profile_mode_str.c_str(),
                     e.what());
        return false;
    }
    return true;
}

uint32_t PowerProfilesHadess::hold_profile(int32_t profile_mode, const std::string &reason)
{
    RETURN_VAL_IF_FALSE(this->profiles_proxy_, -1);

    auto profile_mode_str = this->porfile_mode_enum2str(profile_mode);
    KLOG_DEBUG("Hold power active profile to %s.", profile_mode_str.c_str());

    try
    {
        auto parameters = g_variant_new("(sss)", profile_mode_str.c_str(), reason.c_str(), "kiran-session-daemon");
        Glib::VariantContainerBase base(parameters, false);
        auto retval = this->profiles_proxy_->call_sync("HoldProfile", base);
        auto v1 = retval.get_child(0);
        return Glib::VariantBase::cast_dynamic<Glib::Variant<uint32_t>>(v1).get();
    }
    catch (const Glib::Error &e)
    {
        KLOG_WARNING("Failed to call HoldProfile: %s", e.what().c_str());
        return -1;
    }
    catch (const std::exception &e)
    {
        KLOG_WARNING("Failed to call HoldProfile: %s", e.what());
        return -1;
    }
    return 0;
}

void PowerProfilesHadess::release_profile(uint32_t cookie)
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
        KLOG_WARNING("Failed to call ReleaseProfile: %s", e.what().c_str());
    }
}

int32_t PowerProfilesHadess::get_active_profile()
{
    RETURN_VAL_IF_FALSE(this->profiles_proxy_, POWER_PROFILE_MODE_PERFORMANCE);

    try
    {
        Glib::VariantBase value;
        this->profiles_proxy_->get_cached_property(value, PROFILES_HADESS_DBUS_PROP_ACTIVE_PROFILE);
        auto profile_mode_str = Glib::VariantBase::cast_dynamic<Glib::Variant<Glib::ustring>>(value).get();
        return this->porfile_mode_str2enum(profile_mode_str);
    }
    catch (const std::exception &e)
    {
        KLOG_WARNING("Failed to get property %s: %s", PROFILES_HADESS_DBUS_PROP_ACTIVE_PROFILE, e.what());
    }
    // 默认返回高性能模式
    return POWER_PROFILE_MODE_PERFORMANCE;
}

std::string PowerProfilesHadess::porfile_mode_enum2str(int32_t profile_mode)
{
    switch (profile_mode)
    {
    case PowerProfileMode::POWER_PROFILE_MODE_SAVER:
        return PROFILES_HADESS_MODE_SAVER;
    case PowerProfileMode::POWER_PROFILE_MODE_BALANCED:
        return PROFILES_HADESS_MODE_BALANCED;
    case PowerProfileMode::POWER_PROFILE_MODE_PERFORMANCE:
        return PROFILES_HADESS_MODE_PERFORMANCE;
    default:
    {
        KLOG_WARNING("Unknown profile mode %d, so return performance as current profile mode.", profile_mode);
        return PROFILES_HADESS_MODE_PERFORMANCE;
    }
    }
}

int32_t PowerProfilesHadess::porfile_mode_str2enum(const std::string &profile_mode_str)
{
    switch (shash(profile_mode_str.c_str()))
    {
    case CONNECT(PROFILES_HADESS_MODE_SAVER, _hash):
        return PowerProfileMode::POWER_PROFILE_MODE_SAVER;
    case CONNECT(PROFILES_HADESS_MODE_BALANCED, _hash):
        return PowerProfileMode::POWER_PROFILE_MODE_BALANCED;
    case CONNECT(PROFILES_HADESS_MODE_PERFORMANCE, _hash):
        return PowerProfileMode::POWER_PROFILE_MODE_PERFORMANCE;

    default:
    {
        KLOG_WARNING("Unknown profile mode %s, so return performance as current profile mode.", profile_mode_str.c_str());
        return PowerProfileMode::POWER_PROFILE_MODE_PERFORMANCE;
    }
    }
}

void PowerProfilesHadess::on_properties_changed(const Gio::DBus::Proxy::MapChangedProperties &changed_properties,
                                                const std::vector<Glib::ustring> &invalidated_properties)
{
    try
    {
        for (auto &iter : changed_properties)
        {
            switch (shash(iter.first.c_str()))
            {
            case CONNECT(PROFILES_HADESS_DBUS_PROP_ACTIVE_PROFILE, _hash):
            {
                auto profile_mode_str = Glib::VariantBase::cast_dynamic<Glib::Variant<Glib::ustring>>(iter.second).get();
                auto profile_mode = this->porfile_mode_str2enum(profile_mode_str);
                this->active_profile_changed_.emit(profile_mode);
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
