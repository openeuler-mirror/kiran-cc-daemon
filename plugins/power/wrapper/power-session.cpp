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

#include "plugins/power/wrapper/power-session.h"

namespace Kiran
{
#define MATE_SESSION_DBUS_NAME "org.gnome.SessionManager"
#define MATE_SESSION_DBUS_OBJECT "/org/gnome/SessionManager"
#define MATE_SESSION_DBUS_INTERFACE "org.gnome.SessionManager"

#define MATE_SESSION_PRECENSE_DBUS_OBJECT "/org/gnome/SessionManager/Presence"
#define MATE_SESSION_PRECENSE_DBUS_INTERFACE "org.gnome.SessionManager.Presence"

enum GsmPresenceStatus
{
    GSM_PRESENCE_STATUS_AVAILABLE = 0,
    GSM_PRESENCE_STATUS_INVISIBLE,
    GSM_PRESENCE_STATUS_BUSY,
    GSM_PRESENCE_STATUS_IDLE,
};

enum GsmInhibitorFlag
{
    GSM_INHIBITOR_FLAG_LOGOUT = 1 << 0,
    GSM_INHIBITOR_FLAG_SWITCH_USER = 1 << 1,
    GSM_INHIBITOR_FLAG_SUSPEND = 1 << 2,
    GSM_INHIBITOR_FLAG_IDLE = 1 << 3
};

PowerSession::PowerSession() : is_idle_(false),
                               is_idle_inhibited_(false),
                               is_suspend_inhibited_(false)
{
}

void PowerSession::init()
{
    try
    {
        this->sm_proxy_ = Gio::DBus::Proxy::create_for_bus_sync(Gio::DBus::BUS_TYPE_SESSION,
                                                                MATE_SESSION_DBUS_NAME,
                                                                MATE_SESSION_DBUS_OBJECT,
                                                                MATE_SESSION_DBUS_INTERFACE);

        this->sm_presence_proxy_ = Gio::DBus::Proxy::create_for_bus_sync(Gio::DBus::BUS_TYPE_SESSION,
                                                                         MATE_SESSION_DBUS_NAME,
                                                                         MATE_SESSION_PRECENSE_DBUS_OBJECT,
                                                                         MATE_SESSION_PRECENSE_DBUS_INTERFACE);
    }
    catch (const Glib::Error& e)
    {
        KLOG_WARNING_POWER("%s", e.what().c_str());
        return;
    }

    this->is_idle_ = this->get_idle();
    this->is_idle_inhibited_ = this->get_inhibited(GSM_INHIBITOR_FLAG_IDLE);
    this->is_suspend_inhibited_ = this->get_inhibited(GSM_INHIBITOR_FLAG_SUSPEND);

    this->sm_proxy_->signal_signal().connect(sigc::mem_fun(this, &PowerSession::on_sm_signal));
    this->sm_presence_proxy_->signal_signal().connect(sigc::mem_fun(this, &PowerSession::on_sm_presence_signal));
}

bool PowerSession::can_suspend()
{
    try
    {
        auto retval = this->sm_proxy_->call_sync("CanSuspend", Glib::VariantContainerBase());
        auto v1 = retval.get_child(0);
        return Glib::VariantBase::cast_dynamic<Glib::Variant<bool>>(v1).get();
    }
    catch (const Glib::Error& e)
    {
        KLOG_WARNING("Failed to call CanSuspend: %s", e.what().c_str());
    }
    catch (const std::exception& e)
    {
        KLOG_WARNING("Failed to get the retval of function CanSuspend: %s", e.what());
    }
    return false;
}

void PowerSession::suspend()
{
    if (!this->can_suspend())
    {
        KLOG_WARNING("The session manager doesn't allow suspend.");
        return;
    }

    try
    {
        this->sm_proxy_->call_sync("Suspend", Glib::VariantContainerBase());
    }
    catch (const Glib::Error& e)
    {
        KLOG_WARNING("Failed to call Suspend: %s", e.what().c_str());
    }
}

bool PowerSession::can_hibernate()
{
    try
    {
        auto retval = this->sm_proxy_->call_sync("CanHibernate", Glib::VariantContainerBase());
        auto v1 = retval.get_child(0);
        return Glib::VariantBase::cast_dynamic<Glib::Variant<bool>>(v1).get();
    }
    catch (const Glib::Error& e)
    {
        KLOG_WARNING("Failed to call CanHibernate: %s", e.what().c_str());
    }
    catch (const std::exception& e)
    {
        KLOG_WARNING("Failed to get the retval of function CanHibernate: %s", e.what());
    }
    return false;
}

void PowerSession::hibernate()
{
    if (!this->can_hibernate())
    {
        KLOG_WARNING("The session manager doesn't allow hibernate.");
        return;
    }

    try
    {
        this->sm_proxy_->call_sync("Hibernate", Glib::VariantContainerBase());
    }
    catch (const Glib::Error& e)
    {
        KLOG_WARNING("Failed to call Hibernate: %s", e.what().c_str());
    }
}

bool PowerSession::can_shutdown()
{
    try
    {
        auto retval = this->sm_proxy_->call_sync("CanShutdown", Glib::VariantContainerBase());
        auto v1 = retval.get_child(0);
        return Glib::VariantBase::cast_dynamic<Glib::Variant<bool>>(v1).get();
    }
    catch (const Glib::Error& e)
    {
        KLOG_WARNING("Failed to call CanShutdown: %s", e.what().c_str());
    }
    catch (const std::exception& e)
    {
        KLOG_WARNING("Failed to get the retval of function CanShutdown: %s", e.what());
    }
    return false;
}

void PowerSession::shutdown()
{
    if (!this->can_shutdown())
    {
        KLOG_WARNING("The session manager doesn't allow shutdown.");
        return;
    }

    try
    {
        this->sm_proxy_->call_sync("Shutdown", Glib::VariantContainerBase());
    }
    catch (const Glib::Error& e)
    {
        KLOG_WARNING("Failed to call shutdown: %s", e.what().c_str());
    }
}

uint32_t PowerSession::get_status()
{
    RETURN_VAL_IF_FALSE(this->sm_presence_proxy_, 0);

    try
    {
        Glib::VariantBase property;
        this->sm_presence_proxy_->get_cached_property(property, "status");
        RETURN_VAL_IF_TRUE(property.gobj() == NULL, 0);
        auto status = Glib::VariantBase::cast_dynamic<Glib::Variant<uint32_t>>(property).get();
        return status;
    }
    catch (const std::exception& e)
    {
        KLOG_WARNING_POWER("%s", e.what());
    }

    return 0;
}

bool PowerSession::get_inhibited(uint32_t flag)
{
    RETURN_VAL_IF_FALSE(this->sm_proxy_, false);

    auto parameters = g_variant_new("(u)", flag);
    Glib::VariantContainerBase base(parameters, false);

    try
    {
        auto retval = this->sm_proxy_->call_sync("IsInhibited", base);
        auto v1 = retval.get_child(0);
        auto is_inhibited = Glib::VariantBase::cast_dynamic<Glib::Variant<bool>>(v1).get();
        return is_inhibited;
    }
    catch (const Glib::Error& e)
    {
        KLOG_WARNING_POWER("%s", e.what().c_str());
    }
    catch (const std::exception& e)
    {
        KLOG_WARNING_POWER("%s", e.what());
    }
    return false;
}

void PowerSession::on_sm_signal(const Glib::ustring& sender_name,
                                const Glib::ustring& signal_name,
                                const Glib::VariantContainerBase& parameters)
{
    KLOG_DEBUG_POWER("Recieve the request of %s from %s.", signal_name.c_str(), sender_name.c_str());

    switch (shash(signal_name.c_str()))
    {
    case "InhibitorAdded"_hash:
    case "InhibitorRemoved"_hash:
        this->on_sm_inhibitor_changed_cb(parameters);
        break;
    default:
        break;
    }
}

void PowerSession::on_sm_inhibitor_changed_cb(const Glib::VariantContainerBase& parameters)
{
    auto is_idle_inhibited = this->get_inhibited(GSM_INHIBITOR_FLAG_IDLE);
    auto is_suspend_inhibited = this->get_inhibited(GSM_INHIBITOR_FLAG_SUSPEND);

    if (is_idle_inhibited != this->is_idle_inhibited_ ||
        is_suspend_inhibited != this->is_suspend_inhibited_)
    {
        this->is_idle_inhibited_ = is_idle_inhibited;
        this->is_suspend_inhibited_ = is_suspend_inhibited;
        this->inhibitor_changed_.emit();
    }
}

void PowerSession::on_sm_presence_signal(const Glib::ustring& sender_name,
                                         const Glib::ustring& signal_name,
                                         const Glib::VariantContainerBase& parameters)
{
    KLOG_DEBUG_POWER("Recieve the request of %s from %s.", signal_name.c_str(), sender_name.c_str());

    switch (shash(signal_name.c_str()))
    {
    case "StatusChanged"_hash:
        this->on_sm_presence_status_changed_cb(parameters);
        break;

    default:
        break;
    }
}

void PowerSession::on_sm_presence_status_changed_cb(const Glib::VariantContainerBase& parameters)
{
    try
    {
        Glib::VariantBase v1;
        parameters.get_child(v1, 0);
        auto status = Glib::VariantBase::cast_dynamic<Glib::Variant<uint32_t>>(v1).get();
        KLOG_DEBUG_POWER("Sm presence status is changed to %u", status);

        bool is_idle = (status == GSM_PRESENCE_STATUS_IDLE);

        if (is_idle != this->is_idle_)
        {
            this->is_idle_ = is_idle;
            this->idle_status_changed_.emit(is_idle);
        }
    }
    catch (const std::exception& e)
    {
        KLOG_WARNING_POWER("%s", e.what());
        return;
    }
}
}  // namespace Kiran