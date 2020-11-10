/*
 * @Author       : tangjie02
 * @Date         : 2020-11-09 17:28:10
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-11-10 09:46:06
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/plugins/bluetooth/bluetooth-agent.cpp
 */
#include "plugins/bluetooth/bluetooth-agent.h"

#include "lib/base/base.h"
#include "plugins/bluetooth/bluez.h"

#define AGENT_OBJECT_PATH "/com/kylinsec/Kiran/SessionDaemon/Bluetooth/Agent"
#define AGENT_CAPABILITY "DisplayYesNo"

namespace Kiran
{
BluetoothAgent::BluetoothAgent()
{
}

void BluetoothAgent::init()
{
    bluez::AgentManager1Proxy::createForBus(Gio::DBus::BUS_TYPE_SYSTEM,
                                            Gio::DBus::PROXY_FLAGS_NONE,
                                            BLUEZ_DBUS_NAME,
                                            BLUEZ_MANAGER_OBJECT_PATH,
                                            sigc::mem_fun(this, &BluetoothAgent::on_agent_manager_ready));
}

void BluetoothAgent::destroy()
{
    RETURN_IF_FALSE(this->agent_manager_proxy_);

    this->agent_manager_proxy_->UnregisterAgent(AGENT_OBJECT_PATH, sigc::mem_fun(this, &BluetoothAgent::on_agent_unregister_ready));
}

void BluetoothAgent::Release(MethodInvocation &invocation)
{
    SETTINGS_PROFILE("");
    invocation.ret();
}

void BluetoothAgent::RequestPinCode(const Glib::DBusObjectPathString &device, MethodInvocation &invocation)
{
    SETTINGS_PROFILE("device: %s.", device.c_str());
    invocation.ret("123456");
}

void BluetoothAgent::DisplayPinCode(const Glib::DBusObjectPathString &device,
                                    const Glib::ustring &pincode,
                                    MethodInvocation &invocation)
{
    SETTINGS_PROFILE("device: %s, pincode: %s.", device.c_str(), pincode.c_str());
    invocation.ret();
}

void BluetoothAgent::RequestPasskey(const Glib::DBusObjectPathString &device, MethodInvocation &invocation)
{
    SETTINGS_PROFILE("device: %s.", device.c_str());
    invocation.ret("123456");
}

void BluetoothAgent::DisplayPasskey(const Glib::DBusObjectPathString &device,
                                    const Glib::ustring &passkey,
                                    guint16 entered,
                                    MethodInvocation &invocation)
{
    SETTINGS_PROFILE("device: %s, passkey: %s, entered: %d.", device.c_str(), passkey.c_str(), entered);
    invocation.ret();
}

void BluetoothAgent::RequestConfirmation(const Glib::DBusObjectPathString &device,
                                         const Glib::ustring &passkey,
                                         MethodInvocation &invocation)
{
    SETTINGS_PROFILE("device: %s, passkey: %s.", device.c_str(), passkey.c_str());
    invocation.ret();
}

void BluetoothAgent::RequestAuthorization(const Glib::DBusObjectPathString &device, MethodInvocation &invocation)
{
    SETTINGS_PROFILE("device: %s.", device.c_str());
    invocation.ret();
}

void BluetoothAgent::AuthorizeService(const Glib::DBusObjectPathString &device,
                                      const Glib::ustring &uuid,
                                      MethodInvocation &invocation)
{
    SETTINGS_PROFILE("device: %s, uuid: %s.", device.c_str(), uuid.c_str());
    invocation.ret();
}

void BluetoothAgent::Cancel(MethodInvocation &invocation)
{
    SETTINGS_PROFILE("");
    invocation.ret();
}

void BluetoothAgent::on_agent_manager_ready(Glib::RefPtr<Gio::AsyncResult> &result)
{
    try
    {
        this->agent_manager_proxy_ = bluez::AgentManager1Proxy::createForBusFinish(result);
    }
    catch (const Glib::Error &e)
    {
        LOG_WARNING("Cannot connect to %s: %s.", BLUEZ_MANAGER_OBJECT_PATH, e.what().c_str());
        return;
    }

    this->agent_manager_proxy_->RegisterAgent(AGENT_OBJECT_PATH, AGENT_CAPABILITY, sigc::mem_fun(this, &BluetoothAgent::on_agent_register_ready));
}

void BluetoothAgent::on_agent_register_ready(Glib::RefPtr<Gio::AsyncResult> &result)
{
    try
    {
        this->agent_manager_proxy_->RegisterAgent_finish(result);
    }
    catch (const Glib::Error &e)
    {
        LOG_WARNING("%s.", e.what().c_str());
        return;
    }

    this->agent_manager_proxy_->RequestDefaultAgent(AGENT_OBJECT_PATH, sigc::mem_fun(this, &BluetoothAgent::on_default_agent_ready));
}

void BluetoothAgent::on_default_agent_ready(Glib::RefPtr<Gio::AsyncResult> &result)
{
    try
    {
        this->agent_manager_proxy_->RegisterAgent_finish(result);
    }
    catch (const Glib::Error &e)
    {
        LOG_WARNING("%s.", e.what().c_str());
        return;
    }
}

void BluetoothAgent::on_agent_unregister_ready(Glib::RefPtr<Gio::AsyncResult> &result)
{
    try
    {
        this->agent_manager_proxy_->UnregisterAgent_finish(result);
    }
    catch (const Glib::Error &e)
    {
        LOG_WARNING("%s.", e.what().c_str());
        return;
    }
}

}  // namespace Kiran