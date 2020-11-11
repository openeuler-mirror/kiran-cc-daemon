/*
 * @Author       : tangjie02
 * @Date         : 2020-11-09 17:28:10
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-11-11 10:43:38
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/plugins/bluetooth/bluetooth-agent.cpp
 */
#include "plugins/bluetooth/bluetooth-agent.h"

#include <glib/gi18n.h>

#include "lib/base/base.h"
#include "plugins/bluetooth/bluetooth-manager.h"
#include "plugins/bluetooth/bluez.h"

#define AGENT_OBJECT_PATH "/com/kylinsec/Kiran/SessionDaemon/Bluetooth/Agent"
#define AGENT_CAPABILITY "DisplayYesNo"
#define REQUEST_TIMEOUT 10000

namespace Kiran
{
BluetoothAgent::BluetoothAgent(BluetoothManager *bluetooth_manager) : bluetooth_manager_(bluetooth_manager),
                                                                      object_register_id_(0)
{
}

void BluetoothAgent::init()
{
    SETTINGS_PROFILE("");

    try
    {
        this->connection_ = Gio::DBus::Connection::get_sync(Gio::DBus::BUS_TYPE_SYSTEM);
    }
    catch (const Glib::Error &e)
    {
        LOG_WARNING("%s.", e.what().c_str());
        return;
    }

    try
    {
        this->object_register_id_ = this->register_object(this->connection_, AGENT_OBJECT_PATH);
    }
    catch (const Glib::Error &e)
    {
        LOG_WARNING("register object_path %s fail: %s.", AGENT_OBJECT_PATH, e.what().c_str());
        return;
    }

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
    this->request_response(sigc::bind(sigc::mem_fun(this, &BluetoothAgent::on_pincode_feeded), invocation.getMessage()),
                           device,
                           invocation);
    this->bluetooth_manager_->RequestPinCode_signal.emit(device);
    return;
}

void BluetoothAgent::DisplayPinCode(const Glib::DBusObjectPathString &device,
                                    const Glib::ustring &pincode,
                                    MethodInvocation &invocation)
{
    SETTINGS_PROFILE("device: %s, pincode: %s.", device.c_str(), pincode.c_str());

    this->bluetooth_manager_->DisplayPinCode_signal.emit(device, pincode);

    invocation.ret();
}

void BluetoothAgent::RequestPasskey(const Glib::DBusObjectPathString &device, MethodInvocation &invocation)
{
    SETTINGS_PROFILE("device: %s.", device.c_str());
    this->request_response(sigc::bind(sigc::mem_fun(this, &BluetoothAgent::on_passkey_feeded), invocation.getMessage()),
                           device,
                           invocation);
    this->bluetooth_manager_->RequestPasskey_signal.emit(device);
    return;
}

void BluetoothAgent::DisplayPasskey(const Glib::DBusObjectPathString &device,
                                    guint32 passkey,
                                    guint16 entered,
                                    MethodInvocation &invocation)
{
    SETTINGS_PROFILE("device: %s, passkey: %d, entered: %d.", device.c_str(), passkey, entered);

    this->bluetooth_manager_->DisplayPasskey_signal.emit(device, passkey, entered);

    invocation.ret();
}

void BluetoothAgent::RequestConfirmation(const Glib::DBusObjectPathString &device,
                                         guint32 passkey,
                                         MethodInvocation &invocation)
{
    SETTINGS_PROFILE("device: %s, passkey: %d.", device.c_str(), passkey);
    this->request_response(sigc::bind(sigc::mem_fun(this, &BluetoothAgent::on_confirmation_feeded), invocation.getMessage()),
                           device,
                           invocation);
    this->bluetooth_manager_->RequestConfirmation_signal.emit(device, passkey);
    return;
}

void BluetoothAgent::RequestAuthorization(const Glib::DBusObjectPathString &device, MethodInvocation &invocation)
{
    SETTINGS_PROFILE("device: %s.", device.c_str());
    this->request_response(sigc::bind(sigc::mem_fun(this, &BluetoothAgent::on_confirmation_feeded), invocation.getMessage()),
                           device,
                           invocation);
    this->bluetooth_manager_->RequestAuthorization_signal.emit(device);
    return;
}

void BluetoothAgent::AuthorizeService(const Glib::DBusObjectPathString &device,
                                      const Glib::ustring &uuid,
                                      MethodInvocation &invocation)
{
    SETTINGS_PROFILE("device: %s, uuid: %s.", device.c_str(), uuid.c_str());
    this->bluetooth_manager_->AuthorizeService_signal.emit(device, uuid);
    invocation.ret();
}

void BluetoothAgent::Cancel(MethodInvocation &invocation)
{
    SETTINGS_PROFILE("");
    if (!this->request_device_.empty())
    {
        this->bluetooth_manager_->Cancel_signal.emit(this->request_device_);
    }
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
    SETTINGS_PROFILE("");
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
    SETTINGS_PROFILE("");
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
    SETTINGS_PROFILE("");
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

void BluetoothAgent::request_response(sigc::slot<void, bool, const std::string &> callback,
                                      const Glib::DBusObjectPathString &device,
                                      MethodInvocation invocation)
{
    if (this->feeded_conn_)
    {
        DBUS_ERROR_REPLY_AND_RET(CCError::ERROR_BLUEZ_REJECTED, _("An incomplete request already exists."));
    }

    this->request_device_ = device;
    this->feeded_conn_ = this->bluetooth_manager_->signal_agent_feeded().connect(callback);
    // 超过10秒钟没有收到feed信号则返回失败
    auto timeout = Glib::MainContext::get_default()->signal_timeout();
    this->feeded_timeout_conn_ = timeout.connect(sigc::bind(sigc::mem_fun(this, &BluetoothAgent::on_feeded_timeout), invocation.getMessage()), REQUEST_TIMEOUT);

    this->canceled_conn_ = this->bluetooth_manager_->Cancel_signal.connect(sigc::bind(sigc::mem_fun(this, &BluetoothAgent::on_canceled), invocation));
}

void BluetoothAgent::on_canceled(const Glib::DBusObjectPathString &device, MethodInvocation invocation)
{
    DBUS_ERROR_REPLY(CCError::ERROR_BLUEZ_CANCELED, _("The request is canceled."));
    on_response_finished();
}

void BluetoothAgent::on_pincode_feeded(bool accept, const std::string &pincode, MethodInvocation invocation)
{
    if (!accept)
    {
        DBUS_ERROR_REPLY(CCError::ERROR_BLUEZ_REJECTED, _("The request is rejected."));
    }
    else
    {
        invocation.ret(pincode);
    }
    on_response_finished();
    return;
}

void BluetoothAgent::on_passkey_feeded(bool accept, const std::string &passkey, MethodInvocation invocation)
{
    if (!accept)
    {
        DBUS_ERROR_REPLY(CCError::ERROR_BLUEZ_REJECTED, _("The request is rejected."));
    }
    else
    {
        uint32_t value = std::strtoul(passkey.c_str(), NULL, 0);
        invocation.ret(value);
    }
    on_response_finished();
    return;
}

void BluetoothAgent::on_confirmation_feeded(bool accept, const std::string &, MethodInvocation invocation)
{
    if (!accept)
    {
        DBUS_ERROR_REPLY(CCError::ERROR_BLUEZ_REJECTED, _("The request is rejected."));
    }
    else
    {
        invocation.ret();
    }
    on_response_finished();
}

bool BluetoothAgent::on_feeded_timeout(MethodInvocation invocation)
{
    DBUS_ERROR_REPLY(CCError::ERROR_BLUEZ_REJECTED, _("The request is rejected."));
    on_response_finished();
    return false;
}

void BluetoothAgent::on_response_finished()
{
    this->request_device_ = std::string();

    if (this->feeded_conn_)
    {
        this->feeded_conn_.disconnect();
    }

    if (this->feeded_timeout_conn_)
    {
        this->feeded_timeout_conn_.disconnect();
    }

    if (this->canceled_conn_)
    {
        this->canceled_conn_.disconnect();
    }
}

}  // namespace Kiran