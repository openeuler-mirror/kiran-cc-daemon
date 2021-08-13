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

#include "plugins/network/network-proxy-manager.h"

#include "network-i.h"

namespace Kiran
{
#define MANUAL_TIMEOUT_MILLISECONDS 100

#define NETWORK_PROXY_SCHEMA_ID "com.kylinsec.kiran.network.proxy"

#define NETWORK_PROXY_SCHEMA_KEY_MODE "mode"
#define NETWORK_PROXY_SCHEMA_KEY_AUTOCONFIG_URL "autoconfig-url"
#define NETWORK_PROXY_SCHEMA_KEY_ENABLE_HTTP_AUTH "enable-http-auth"
#define NETWORK_PROXY_SCHEMA_KEY_HTTP_AUTH_USER "http-auth-user"
#define NETWORK_PROXY_SCHEMA_KEY_HTTP_AUTH_PASSWD "http-auth-password"
#define NETWORK_PROXY_SCHEMA_KEY_HTTP_HOST "http-host"
#define NETWORK_PROXY_SCHEMA_KEY_HTTP_PORT "http-port"
#define NETWORK_PROXY_SCHEMA_KEY_HTTPS_HOST "https-host"
#define NETWORK_PROXY_SCHEMA_KEY_HTTPS_PORT "https-port"
#define NETWORK_PROXY_SCHEMA_KEY_FTP_HOST "ftp-host"
#define NETWORK_PROXY_SCHEMA_KEY_FTP_PORT "ftp-port"
#define NETWORK_PROXY_SCHEMA_KEY_SOCKS_HOST "socks-host"
#define NETWORK_PROXY_SCHEMA_KEY_SOCKS_PORT "socks-port"

NetworkProxyManager::NetworkProxyManager()
{
    this->proxy_settings_ = Gio::Settings::create(NETWORK_PROXY_SCHEMA_ID);
}

NetworkProxyManager* NetworkProxyManager::instance_ = nullptr;
void NetworkProxyManager::global_init()
{
    instance_ = new NetworkProxyManager();
    instance_->init();
}

void NetworkProxyManager::SetMode(gint32 mode,
                                  MethodInvocation& invocation)
{
    KLOG_PROFILE("Mode: %d.", mode);

    if (mode < NETWORK_PROXY_MODE_NONE || mode >= NETWORK_PROXY_MODE_LAST)
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_NETWORK_PROXY_MODE_INVALID);
    }

    if (!this->mode_set(mode))
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_NETWORK_PROXY_SET_MODE_FAILED);
    }

    invocation.ret();
}

void NetworkProxyManager::SetManualProxy(const Glib::ustring& options,
                                         MethodInvocation& invocation)
{
    KLOG_PROFILE("Options: %s.", options.c_str());

    if (NETWORK_PROXY_MODE_MANUAL != this->mode_get())
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_NETWORK_PROXY_CURRENT_MODE_NOT_MANUAL);
    }

    Json::Value values;
    Json::Reader reader;

    bool result = reader.parse(options, values);
    if (!result)
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_NETWORK_PROXY_JSON_FORMAT_FAILED);
    }

    try
    {
        for (auto name : values.getMemberNames())
        {
            switch (shash(name.c_str()))
            {
            case CONNECT(NETWORK_PROXY_MJK_HTTP, _hash):
            {
                save_http_settings(values[NETWORK_PROXY_MJK_HTTP]);
            }
            break;
            case CONNECT(NETWORK_PROXY_MJK_HTTPS, _hash):
            {
                save_https_settings(values[NETWORK_PROXY_MJK_HTTPS]);
            }
            break;
            case CONNECT(NETWORK_PROXY_MJK_FTP, _hash):
            {
                save_ftp_settings(values[NETWORK_PROXY_MJK_FTP]);
            }
            break;
            case CONNECT(NETWORK_PROXY_MJK_SOCKS, _hash):
            {
                save_socks_settings(values[NETWORK_PROXY_MJK_SOCKS]);
            }
            break;
            default:
                break;
            }
        }
    }
    catch (const std::exception& e)
    {
        KLOG_WARNING("%s.", e.what());
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_NETWORK_PROXY_SET_MANUAL_PROXY_FAILED);
    }

    invocation.ret();
}

void NetworkProxyManager::GetManualProxy(MethodInvocation& invocation)
{
    Json::Value values;
    Json::FastWriter writer;

    try
    {
        values[NETWORK_PROXY_MJK_HTTP][NETWORK_PROXY_MJK_ENABLE_HTTP_AUTH] = this->proxy_settings_->get_boolean(NETWORK_PROXY_SCHEMA_KEY_ENABLE_HTTP_AUTH);
        values[NETWORK_PROXY_MJK_HTTP][NETWORK_PROXY_MJK_HTTP_AUTH_USER] = std::string(this->proxy_settings_->get_string(NETWORK_PROXY_SCHEMA_KEY_HTTP_AUTH_USER));
        values[NETWORK_PROXY_MJK_HTTP][NETWORK_PROXY_MJK_HTTP_AUTH_PASSWD] = std::string(this->proxy_settings_->get_string(NETWORK_PROXY_SCHEMA_KEY_HTTP_AUTH_PASSWD));

        values[NETWORK_PROXY_MJK_HTTP][NETWORK_PROXY_MJK_HOST] = std::string(this->proxy_settings_->get_string(NETWORK_PROXY_SCHEMA_KEY_HTTP_HOST));
        values[NETWORK_PROXY_MJK_HTTP][NETWORK_PROXY_MJK_PORT] = this->proxy_settings_->get_int(NETWORK_PROXY_SCHEMA_KEY_HTTP_PORT);

        values[NETWORK_PROXY_MJK_HTTPS][NETWORK_PROXY_MJK_HOST] = std::string(this->proxy_settings_->get_string(NETWORK_PROXY_SCHEMA_KEY_HTTPS_HOST));
        values[NETWORK_PROXY_MJK_HTTPS][NETWORK_PROXY_MJK_PORT] = this->proxy_settings_->get_int(NETWORK_PROXY_SCHEMA_KEY_HTTPS_PORT);

        values[NETWORK_PROXY_MJK_FTP][NETWORK_PROXY_MJK_HOST] = std::string(this->proxy_settings_->get_string(NETWORK_PROXY_SCHEMA_KEY_FTP_HOST));
        values[NETWORK_PROXY_MJK_FTP][NETWORK_PROXY_MJK_PORT] = this->proxy_settings_->get_int(NETWORK_PROXY_SCHEMA_KEY_FTP_PORT);

        values[NETWORK_PROXY_MJK_SOCKS][NETWORK_PROXY_MJK_HOST] = std::string(this->proxy_settings_->get_string(NETWORK_PROXY_SCHEMA_KEY_SOCKS_HOST));
        values[NETWORK_PROXY_MJK_SOCKS][NETWORK_PROXY_MJK_PORT] = this->proxy_settings_->get_int(NETWORK_PROXY_SCHEMA_KEY_SOCKS_PORT);
    }
    catch (const std::exception& e)
    {
        KLOG_WARNING("%s.", e.what());
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_NETWORK_PROXY_GET_MANUAL_PROXY_FAILED);
    }

    auto result = writer.write(values);
    invocation.ret(result);
}

void NetworkProxyManager::SetAutoProxy(const Glib::ustring& url,
                                       MethodInvocation& invocation)
{
    KLOG_PROFILE("Auto proxy url: %s.", url.c_str());

    if (NETWORK_PROXY_MODE_AUTO != this->mode_get())
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_NETWORK_PROXY_CURRENT_MODE_NOT_AUTO);
    }

    if (!this->proxy_url_set(url))
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_NETWORK_PROXY_SET_AUTO_PROXY_URL_FAILED);
    }

    invocation.ret();
}

bool NetworkProxyManager::mode_setHandler(gint32 value)
{
    return this->proxy_settings_->set_enum(NETWORK_PROXY_SCHEMA_KEY_MODE, value);
}

gint32 NetworkProxyManager::mode_get()
{
    return this->proxy_settings_->get_enum(NETWORK_PROXY_SCHEMA_KEY_MODE);
}

bool NetworkProxyManager::proxy_url_setHandler(const Glib::ustring& value)
{
    this->proxy_settings_->set_string(NETWORK_PROXY_SCHEMA_KEY_AUTOCONFIG_URL, value);
    return true;
}

Glib::ustring NetworkProxyManager::proxy_url_get()
{
    return this->proxy_settings_->get_string(NETWORK_PROXY_SCHEMA_KEY_AUTOCONFIG_URL);
}

void NetworkProxyManager::init()
{
    this->proxy_settings_->signal_changed().connect(sigc::mem_fun(this, &NetworkProxyManager::on_settings_changed));

    this->dbus_connect_id_ = Gio::DBus::own_name(Gio::DBus::BUS_TYPE_SESSION,
                                                 NETWORK_PROXY_DBUS_NAME,
                                                 sigc::mem_fun(this, &NetworkProxyManager::on_bus_acquired),
                                                 sigc::mem_fun(this, &NetworkProxyManager::on_name_acquired),
                                                 sigc::mem_fun(this, &NetworkProxyManager::on_name_lost));
}

void NetworkProxyManager::save_http_settings(const Json::Value& values)
{
    try
    {
        for (auto name : values.getMemberNames())
        {
            switch (shash(name.c_str()))
            {
            case CONNECT(NETWORK_PROXY_MJK_ENABLE_HTTP_AUTH, _hash):
                this->proxy_settings_->set_boolean(NETWORK_PROXY_SCHEMA_KEY_ENABLE_HTTP_AUTH, values[NETWORK_PROXY_MJK_ENABLE_HTTP_AUTH].asBool());
                break;
            case CONNECT(NETWORK_PROXY_MJK_HTTP_AUTH_USER, _hash):
                this->proxy_settings_->set_string(NETWORK_PROXY_SCHEMA_KEY_HTTP_AUTH_USER, values[NETWORK_PROXY_MJK_HTTP_AUTH_USER].asString());
                break;
            case CONNECT(NETWORK_PROXY_MJK_HTTP_AUTH_PASSWD, _hash):
                this->proxy_settings_->set_string(NETWORK_PROXY_SCHEMA_KEY_HTTP_AUTH_PASSWD, values[NETWORK_PROXY_MJK_HTTP_AUTH_PASSWD].asString());
                break;
            case CONNECT(NETWORK_PROXY_MJK_HOST, _hash):
                this->proxy_settings_->set_string(NETWORK_PROXY_SCHEMA_KEY_HTTP_HOST, values[NETWORK_PROXY_MJK_HOST].asString());
                break;
            case CONNECT(NETWORK_PROXY_MJK_PORT, _hash):
                this->proxy_settings_->set_int(NETWORK_PROXY_SCHEMA_KEY_HTTP_PORT, values[NETWORK_PROXY_MJK_PORT].asInt());
                break;
            default:
                break;
            }
        }
    }
    catch (const std::exception& e)
    {
        KLOG_WARNING("%s.", e.what());
    }
}

void NetworkProxyManager::save_https_settings(const Json::Value& values)
{
    try
    {
        for (auto name : values.getMemberNames())
        {
            switch (shash(name.c_str()))
            {
            case CONNECT(NETWORK_PROXY_MJK_HOST, _hash):
                this->proxy_settings_->set_string(NETWORK_PROXY_SCHEMA_KEY_HTTPS_HOST, values[NETWORK_PROXY_MJK_HOST].asString());
                break;
            case CONNECT(NETWORK_PROXY_MJK_PORT, _hash):
                this->proxy_settings_->set_int(NETWORK_PROXY_SCHEMA_KEY_HTTPS_PORT, values[NETWORK_PROXY_MJK_PORT].asInt());
                break;
            default:
                break;
            }
        }
    }
    catch (const std::exception& e)
    {
        KLOG_WARNING("%s.", e.what());
    }
}

void NetworkProxyManager::save_ftp_settings(const Json::Value& values)
{
    try
    {
        for (auto name : values.getMemberNames())
        {
            switch (shash(name.c_str()))
            {
            case CONNECT(NETWORK_PROXY_MJK_HOST, _hash):
                this->proxy_settings_->set_string(NETWORK_PROXY_SCHEMA_KEY_FTP_HOST, values[NETWORK_PROXY_MJK_HOST].asString());
                break;
            case CONNECT(NETWORK_PROXY_MJK_PORT, _hash):
                this->proxy_settings_->set_int(NETWORK_PROXY_SCHEMA_KEY_FTP_PORT, values[NETWORK_PROXY_MJK_PORT].asInt());
                break;
            default:
                break;
            }
        }
    }
    catch (const std::exception& e)
    {
        KLOG_WARNING("%s.", e.what());
    }
}

void NetworkProxyManager::save_socks_settings(const Json::Value& values)
{
    try
    {
        for (auto name : values.getMemberNames())
        {
            switch (shash(name.c_str()))
            {
            case CONNECT(NETWORK_PROXY_MJK_HOST, _hash):
                this->proxy_settings_->set_string(NETWORK_PROXY_SCHEMA_KEY_SOCKS_HOST, values[NETWORK_PROXY_MJK_HOST].asString());
                break;
            case CONNECT(NETWORK_PROXY_MJK_PORT, _hash):
                this->proxy_settings_->set_int(NETWORK_PROXY_SCHEMA_KEY_SOCKS_PORT, values[NETWORK_PROXY_MJK_PORT].asInt());
                break;
            default:
                break;
            }
        }
    }
    catch (const std::exception& e)
    {
        KLOG_WARNING("%s.", e.what());
    }
}

void NetworkProxyManager::on_settings_changed(const Glib::ustring& key)
{
    switch (shash(key.c_str()))
    {
    case CONNECT(NETWORK_PROXY_SCHEMA_KEY_ENABLE_HTTP_AUTH, _hash):
    case CONNECT(NETWORK_PROXY_SCHEMA_KEY_HTTP_AUTH_USER, _hash):
    case CONNECT(NETWORK_PROXY_SCHEMA_KEY_HTTP_AUTH_PASSWD, _hash):
    case CONNECT(NETWORK_PROXY_SCHEMA_KEY_HTTP_HOST, _hash):
    case CONNECT(NETWORK_PROXY_SCHEMA_KEY_HTTP_PORT, _hash):
    case CONNECT(NETWORK_PROXY_SCHEMA_KEY_HTTPS_HOST, _hash):
    case CONNECT(NETWORK_PROXY_SCHEMA_KEY_HTTPS_PORT, _hash):
    case CONNECT(NETWORK_PROXY_SCHEMA_KEY_FTP_HOST, _hash):
    case CONNECT(NETWORK_PROXY_SCHEMA_KEY_FTP_PORT, _hash):
    case CONNECT(NETWORK_PROXY_SCHEMA_KEY_SOCKS_HOST, _hash):
    case CONNECT(NETWORK_PROXY_SCHEMA_KEY_SOCKS_PORT, _hash):
    {
        RETURN_IF_TRUE(this->timeout_manual_proxy_);

        auto timeout = Glib::MainContext::get_default()->signal_timeout();
        this->timeout_manual_proxy_ = timeout.connect(sigc::bind(sigc::mem_fun(this, &NetworkProxyManager::on_manual_proxy_changed), key), MANUAL_TIMEOUT_MILLISECONDS);
    }
    break;
    default:
        break;
    }
}

bool NetworkProxyManager::on_manual_proxy_changed(const Glib::ustring& key)
{
    KLOG_PROFILE("Key: %s.", key.c_str());

    this->ManualProxyChanged_signal.emit('0');
    return false;
}

void NetworkProxyManager::on_bus_acquired(const Glib::RefPtr<Gio::DBus::Connection>& connect, Glib::ustring name)
{
    KLOG_PROFILE("Name: %s.", name.c_str());
    if (!connect)
    {
        KLOG_WARNING("Failed to connect dbus name: %s.", name.c_str());
        return;
    }

    try
    {
        this->object_register_id_ = this->register_object(connect, NETWORK_PROXY_OBJECT_PATH);
    }
    catch (const Glib::Error& e)
    {
        KLOG_WARNING("Register object_path %s fail: %s.", NETWORK_PROXY_OBJECT_PATH, e.what().c_str());
    }
}

void NetworkProxyManager::on_name_acquired(const Glib::RefPtr<Gio::DBus::Connection>& connect, Glib::ustring name)
{
    KLOG_DEBUG("Success to register dbus name: %s.", name.c_str());
}

void NetworkProxyManager::on_name_lost(const Glib::RefPtr<Gio::DBus::Connection>& connect, Glib::ustring name)
{
    KLOG_WARNING("Failed to register dbus name: %s.", name.c_str());
}

}  // namespace Kiran
