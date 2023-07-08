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

#include "plugins/power/wrapper/power-profiles-tuned.h"
#include "power-i.h"

namespace Kiran
{
#define PROFILES_TUNED_MODE_SAVER "powersave"
#define PROFILES_TUNED_MODE_BALANCED "balanced"
#define PROFILES_TUNED_MODE_PERFORMANCE "throughput-performance"

#define PROFILES_TUNED_DBUS_NAME "com.redhat.tuned"
#define PROFILES_TUNED_DBUS_OBJECT_PATH "/Tuned"
#define PROFILES_TUNED_DBUS_INTERFACE "com.redhat.tuned.control"

PowerProfilesTuned::PowerProfilesTuned()
{
    try
    {
        this->profiles_proxy_ = Gio::DBus::Proxy::create_for_bus_sync(Gio::DBus::BUS_TYPE_SYSTEM,
                                                                      PROFILES_TUNED_DBUS_NAME,
                                                                      PROFILES_TUNED_DBUS_OBJECT_PATH,
                                                                      PROFILES_TUNED_DBUS_INTERFACE);
    }
    catch (const Glib::Error &e)
    {
        KLOG_WARNING("Failed to create bus sync: %s", e.what().c_str());
        return;
    }
}

void PowerProfilesTuned::init()
{
    this->profiles_proxy_->signal_signal().connect(sigc::mem_fun(this, &PowerProfilesTuned::on_profile_signal));
}

bool PowerProfilesTuned::switch_profile(int32_t profile_mode)
{
    auto retval = this->hold_profile(profile_mode, std::string());
    return (retval >= 0);
}

uint32_t PowerProfilesTuned::hold_profile(int32_t profile_mode, const std::string &)
{
    RETURN_VAL_IF_FALSE(this->profiles_proxy_, 0);

    auto profile_mode_str = this->porfile_mode_enum2str(profile_mode);
    KLOG_DEBUG("Hold power active profile to %s.", profile_mode_str.c_str());

    try
    {
        auto parameters = g_variant_new("(s)", profile_mode_str.c_str());
        Glib::VariantContainerBase base(parameters, false);
        auto retval = this->profiles_proxy_->call_sync("switch_profile", base);

        auto ret_parameters = retval.get_child(0);
        auto ret_parameters_tuple = Glib::VariantBase::cast_dynamic<Glib::Variant<std::tuple<bool, Glib::ustring>>>(ret_parameters);
        auto successed = ret_parameters_tuple.get_child<bool>(0);
        auto reason = ret_parameters_tuple.get_child<Glib::ustring>(1);

        if (!successed)
        {
            KLOG_WARNING("Failed to call switch_profile: %s.", reason.c_str());
            return -1;
        }
    }
    catch (const Glib::Error &e)
    {
        KLOG_WARNING("Failed to call switch_profile: %s", e.what().c_str());
        return -1;
    }
    catch (const std::exception &e)
    {
        KLOG_WARNING("Failed to call switch_profile: %s", e.what());
        return -1;
    }

    return 0;
}

int32_t PowerProfilesTuned::get_active_profile()
{
    RETURN_VAL_IF_FALSE(this->profiles_proxy_, POWER_PROFILE_MODE_PERFORMANCE);

    std::string profile_mode_str;
    try
    {
        auto retval = this->profiles_proxy_->call_sync("active_profile", Glib::VariantContainerBase());
        auto v1 = retval.get_child(0);
        profile_mode_str = Glib::VariantBase::cast_dynamic<Glib::Variant<Glib::ustring>>(v1).get();
    }
    catch (const Glib::Error &e)
    {
        KLOG_WARNING("Failed to call active_profile: %s", e.what().c_str());
    }
    catch (const std::exception &e)
    {
        KLOG_WARNING("Failed to call active_profile: %s", e.what());
    }

    return this->porfile_mode_str2enum(profile_mode_str);
}

std::string PowerProfilesTuned::porfile_mode_enum2str(int32_t profile_mode)
{
    switch (profile_mode)
    {
    case PowerProfileMode::POWER_PROFILE_MODE_SAVER:
        return PROFILES_TUNED_MODE_SAVER;
    case PowerProfileMode::POWER_PROFILE_MODE_BALANCED:
        return PROFILES_TUNED_MODE_BALANCED;
    case PowerProfileMode::POWER_PROFILE_MODE_PERFORMANCE:
        return PROFILES_TUNED_MODE_PERFORMANCE;
    default:
    {
        KLOG_WARNING("Unknown profile mode %d, so return performance as current profile mode.", profile_mode);
        return PROFILES_TUNED_MODE_PERFORMANCE;
    }
    }
}

int32_t PowerProfilesTuned::porfile_mode_str2enum(const std::string &profile_mode_str)
{
    switch (shash(profile_mode_str.c_str()))
    {
    case CONNECT(PROFILES_TUNED_MODE_SAVER, _hash):
        return PowerProfileMode::POWER_PROFILE_MODE_SAVER;
    case CONNECT(PROFILES_TUNED_MODE_BALANCED, _hash):
        return PowerProfileMode::POWER_PROFILE_MODE_BALANCED;
    case CONNECT(PROFILES_TUNED_MODE_PERFORMANCE, _hash):
        return PowerProfileMode::POWER_PROFILE_MODE_PERFORMANCE;

    default:
    {
        KLOG_WARNING("Unknown profile mode %s, so return performance as current profile mode.", profile_mode_str.c_str());
        return PowerProfileMode::POWER_PROFILE_MODE_PERFORMANCE;
    }
    }
}

void PowerProfilesTuned::on_profile_signal(const Glib::ustring &sender_name,
                                           const Glib::ustring &signal_name,
                                           const Glib::VariantContainerBase &parameters)
{
    switch (shash(signal_name.c_str()))
    {
    case "profile_changed"_hash:
    {
        try
        {
            Glib::VariantContainerBase v1;
            Glib::VariantContainerBase v2;
            Glib::VariantContainerBase v3;
            parameters.get_child(v1, 0);
            parameters.get_child(v2, 1);
            parameters.get_child(v3, 2);

            auto profiled_mode_str = Glib::VariantBase::cast_dynamic<Glib::Variant<Glib::ustring>>(v1).get();
            auto successed = Glib::VariantBase::cast_dynamic<Glib::Variant<bool>>(v2).get();
            auto reason = Glib::VariantBase::cast_dynamic<Glib::Variant<Glib::ustring>>(v3).get();

            if (!successed)
            {
                KLOG_WARNING("Failed to call switch_profile: %s.", reason.c_str());
            }
            else
            {
                this->active_profile_changed_.emit(this->porfile_mode_str2enum(profiled_mode_str));
            }
        }
        catch (const std::exception &e)
        {
            KLOG_WARNING("%s.", e.what());
        }
        break;
    }
    default:
        break;
    }
}

}  // namespace Kiran
