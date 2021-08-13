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
        KLOG_WARNING("%s", e.what().c_str());
        return;
    }

    this->is_idle_ = this->get_idle();
    this->is_idle_inhibited_ = this->get_inhibited(GSM_INHIBITOR_FLAG_IDLE);
    this->is_suspend_inhibited_ = this->get_inhibited(GSM_INHIBITOR_FLAG_SUSPEND);

    this->sm_proxy_->signal_signal().connect(sigc::mem_fun(this, &PowerSession::on_sm_signal));
    this->sm_presence_proxy_->signal_signal().connect(sigc::mem_fun(this, &PowerSession::on_sm_presence_signal));
}

uint32_t PowerSession::get_status()
{
    KLOG_PROFILE("");
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
        KLOG_WARNING("%s", e.what());
    }

    return 0;
}

bool PowerSession::get_inhibited(uint32_t flag)
{
    KLOG_PROFILE("flag: %u", flag);
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
        KLOG_WARNING("%s", e.what().c_str());
    }
    catch (const std::exception& e)
    {
        KLOG_WARNING("%s", e.what());
    }
    return false;
}

void PowerSession::on_sm_signal(const Glib::ustring& sender_name,
                                const Glib::ustring& signal_name,
                                const Glib::VariantContainerBase& parameters)
{
    KLOG_PROFILE("sender_name: %s, signal_name: %s.", sender_name.c_str(), signal_name.c_str());

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
    KLOG_PROFILE("");

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
    KLOG_PROFILE("sender_name: %s, signal_name: %s.", sender_name.c_str(), signal_name.c_str());

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
    KLOG_PROFILE("");

    try
    {
        Glib::VariantBase v1;
        parameters.get_child(v1, 0);
        auto status = Glib::VariantBase::cast_dynamic<Glib::Variant<uint32_t>>(v1).get();
        KLOG_DEBUG("status: %u", status);

        bool is_idle = (status == GSM_PRESENCE_STATUS_IDLE);

        if (is_idle != this->is_idle_)
        {
            this->is_idle_ = is_idle;
            this->idle_status_changed_.emit(is_idle);
        }
    }
    catch (const std::exception& e)
    {
        KLOG_WARNING("%s", e.what());
        return;
    }
}
}  // namespace Kiran