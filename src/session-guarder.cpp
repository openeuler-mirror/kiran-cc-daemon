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

#include "src/session-guarder.h"

namespace Kiran
{
#define MATE_SESSION_DBUS_NAME "org.gnome.SessionManager"
#define MATE_SESSION_DBUS_OBJECT "/org/gnome/SessionManager"
#define MATE_SESSION_DBUS_INTERFACE "org.gnome.SessionManager"
#define MATE_SESSION_PRIVATE_DBUS_INTERFACE "org.gnome.SessionManager.ClientPrivate"

SessionGuarder::SessionGuarder()
{
}

SessionGuarder* SessionGuarder::instance_ = nullptr;

void SessionGuarder::global_init()
{
    instance_ = new SessionGuarder();
    instance_->init();
}

void SessionGuarder::init()
{
    g_autofree gchar* client_id = NULL;

    try
    {
        this->sm_proxy_ = Gio::DBus::Proxy::create_for_bus_sync(Gio::DBus::BUS_TYPE_SESSION,
                                                                MATE_SESSION_DBUS_NAME,
                                                                MATE_SESSION_DBUS_OBJECT,
                                                                MATE_SESSION_DBUS_INTERFACE);
    }
    catch (const Glib::Error& e)
    {
        KLOG_WARNING("%s", e.what().c_str());
        return;
    }

    if (!this->sm_proxy_)
    {
        KLOG_WARNING("Cannot create dbus proxy for %s:%s.", MATE_SESSION_DBUS_NAME, MATE_SESSION_DBUS_OBJECT);
        return;
    }

    // 因为设置了开机启动，所以需要向SessionManager发起注册请求，否则SessionManager会长时间等待导致进入桌面延时
    auto startup_id = g_getenv("DESKTOP_AUTOSTART_ID");
    if (startup_id != NULL && *startup_id != '\0')
    {
        try
        {
            auto retval = this->sm_proxy_->call_sync("RegisterClient",
                                                     Glib::VariantContainerBase(g_variant_new("(ss)", "kiran-session-daemon", startup_id)));

            g_variant_get(retval.gobj(), "(o)", &client_id);

            this->sm_private_proxy_ = Gio::DBus::Proxy::create_for_bus_sync(Gio::DBus::BUS_TYPE_SESSION,
                                                                            MATE_SESSION_DBUS_NAME,
                                                                            client_id,
                                                                            MATE_SESSION_PRIVATE_DBUS_INTERFACE);

            if (!this->sm_private_proxy_)
            {
                KLOG_WARNING("Cannot create dbus proxy for %s:%s.", MATE_SESSION_DBUS_NAME, client_id);
            }

            this->sm_private_proxy_->signal_signal().connect(sigc::mem_fun(this, &SessionGuarder::on_sm_signal));
        }
        catch (const Glib::Error& e)
        {
            KLOG_WARNING("Failed to register client '%s': %s", startup_id, e.what().c_str());
            return;
        }
    }

    // watch_for_term_signal(manager);
}

void SessionGuarder::on_sm_signal(const Glib::ustring& sender_name, const Glib::ustring& signal_name, const Glib::VariantContainerBase& parameters)
{
    switch (shash(signal_name.c_str()))
    {
    case "QueryEndSession"_hash:
        this->on_session_query_end();
        break;
    case "EndSession"_hash:
        this->on_session_end();
        break;
    default:
        break;
    }
}
void SessionGuarder::on_session_query_end()
{
    try
    {
        this->sm_private_proxy_->call_sync("EndSessionResponse",
                                           Glib::VariantContainerBase(g_variant_new("(bs)", TRUE, NULL)));
    }
    catch (const Glib::Error& e)
    {
        KLOG_WARNING("Failed to send end session response: %s", e.what().c_str());
    }
}

void SessionGuarder::on_session_end()
{
    // 发送会话退出的信号，让资源释放等逻辑先处理完，然后再回复dbus消息，否则可能会导致资源释放到一半进程直接被干掉了
    this->session_end_.emit();

    try
    {
        this->sm_private_proxy_->call_sync("EndSessionResponse",
                                           Glib::VariantContainerBase(g_variant_new("(bs)", TRUE, NULL)));
    }
    catch (const Glib::Error& e)
    {
        KLOG_WARNING("Failed to send end session response: %s", e.what().c_str());
    }
}
}  // namespace Kiran