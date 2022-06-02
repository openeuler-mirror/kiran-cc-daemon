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
 * Author:     meizhigang <meizhigang@kylinos.com.cn>
 */

#pragma once

#include <json/json.h>
#include <network_proxy_dbus_stub.h>
#include "lib/base/base.h"

namespace Kiran
{
class NetworkProxyManager : public SessionDaemon::Network::ProxyStub
{
public:
    NetworkProxyManager();
    virtual ~NetworkProxyManager(){};

    static NetworkProxyManager *get_instance() { return instance_; };

    static void global_init();

    static void global_deinit() { delete instance_; };

protected:
    virtual void SetMode(
        gint32 mode,
        MethodInvocation &invocation);

    virtual void SetManualProxy(const Glib::ustring &options,
                                MethodInvocation &invocation);

    virtual void GetManualProxy(MethodInvocation &invocation);

    virtual void SetAutoProxy(const Glib::ustring &url,
                              MethodInvocation &invocation);

    virtual bool mode_setHandler(gint32 value);
    virtual gint32 mode_get();

    virtual bool proxy_url_setHandler(const Glib::ustring &value);
    virtual Glib::ustring proxy_url_get();

private:
    void init();

    void save_http_settings(const Json::Value &values);
    void save_https_settings(const Json::Value &values);
    void save_ftp_settings(const Json::Value &values);
    void save_socks_settings(const Json::Value &values);

    void on_settings_changed(const Glib::ustring &key);
    bool on_manual_proxy_changed(const Glib::ustring &key);

    void on_bus_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name);
    void on_name_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name);
    void on_name_lost(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name);

private:
    static NetworkProxyManager *instance_;

    Glib::RefPtr<Gio::Settings> proxy_settings_;

    sigc::connection timeout_manual_proxy_;

    // dbus
    uint32_t dbus_connect_id_;
    uint32_t object_register_id_;
};
}  // namespace Kiran
