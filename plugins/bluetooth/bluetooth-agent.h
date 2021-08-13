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

#pragma once

#include <bluez_agent_dbus_stub.h>
#include <bluez_agent_manager_dbus_proxy.h>

namespace Kiran
{
class BluetoothManager;

class BluetoothAgent : public bluez::Agent1Stub
{
public:
    BluetoothAgent(BluetoothManager *bluetooth_manager);
    virtual ~BluetoothAgent(){};

    void init();

    void destroy();

protected:
    virtual void Release(MethodInvocation &invocation);
    virtual void RequestPinCode(const Glib::DBusObjectPathString &device, MethodInvocation &invocation);
    virtual void DisplayPinCode(const Glib::DBusObjectPathString &device,
                                const Glib::ustring &pincode,
                                MethodInvocation &invocation);
    virtual void RequestPasskey(const Glib::DBusObjectPathString &device, MethodInvocation &invocation);
    virtual void DisplayPasskey(const Glib::DBusObjectPathString &device,
                                guint32 passkey,
                                guint16 entered,
                                MethodInvocation &invocation);
    virtual void RequestConfirmation(const Glib::DBusObjectPathString &device,
                                     guint32 passkey,
                                     MethodInvocation &invocation);
    virtual void RequestAuthorization(const Glib::DBusObjectPathString &device, MethodInvocation &invocation);
    virtual void AuthorizeService(const Glib::DBusObjectPathString &device,
                                  const Glib::ustring &uuid,
                                  MethodInvocation &invocation);
    virtual void Cancel(MethodInvocation &invocation);

private:
    void on_agent_manager_ready(Glib::RefPtr<Gio::AsyncResult> &result);
    void on_agent_register_ready(Glib::RefPtr<Gio::AsyncResult> &result);
    void on_default_agent_ready(Glib::RefPtr<Gio::AsyncResult> &result);
    void on_agent_unregister_ready(Glib::RefPtr<Gio::AsyncResult> &result);

    void request_response(sigc::slot<void, bool, const std::string &> callback, const Glib::DBusObjectPathString &device, MethodInvocation invocation);
    void on_canceled(const Glib::DBusObjectPathString &device, MethodInvocation invocation);
    void on_pincode_feeded(bool accept, const std::string &pincode, MethodInvocation invocation);
    void on_passkey_feeded(bool accept, const std::string &passkey, MethodInvocation invocation);
    void on_confirmation_feeded(bool accept, const std::string &, MethodInvocation invocation);
    bool on_feeded_timeout(MethodInvocation invocation);
    void on_response_finished();

private:
    Glib::RefPtr<bluez::AgentManager1Proxy> agent_manager_proxy_;

    BluetoothManager *bluetooth_manager_;
    sigc::connection feeded_conn_;
    sigc::connection feeded_timeout_conn_;
    sigc::connection canceled_conn_;
    Glib::DBusObjectPathString request_device_;

    Glib::RefPtr<Gio::DBus::Connection> connection_;
    uint32_t object_register_id_;
};
}  // namespace Kiran