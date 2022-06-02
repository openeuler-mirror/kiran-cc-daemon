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