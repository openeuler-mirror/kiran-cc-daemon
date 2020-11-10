/*
 * @Author       : tangjie02
 * @Date         : 2020-11-09 17:28:07
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-11-10 09:42:46
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/plugins/bluetooth/bluetooth-agent.h
 */
#pragma once

#include <bluez_agent_dbus_stub.h>
#include <bluez_agent_manager_dbus_proxy.h>

namespace Kiran
{
class BluetoothAgent : public bluez::Agent1Stub
{
public:
    BluetoothAgent();
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

private:
    Glib::RefPtr<bluez::AgentManager1Proxy> agent_manager_proxy_;

    Glib::RefPtr<Gio::DBus::Connection> connection_;
    uint32_t object_register_id_;
};
}  // namespace Kiran