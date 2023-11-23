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

#include "src/settings-manager.h"

#include "common.h"
#include "config.h"
#include "lib/base/base.h"

namespace Kiran
{
#define PLUGIN_CONFIG_NAME "plugin_options"

SettingsManager::SettingsManager() : dbus_connect_id_(0),
                                     object_register_id_(0)
{
}

SettingsManager::~SettingsManager()
{
    if (this->dbus_connect_id_)
    {
        Gio::DBus::unown_name(this->dbus_connect_id_);
    }

    this->deactivate_plugins();
}

SettingsManager* SettingsManager::instance_ = nullptr;
void SettingsManager::global_init()
{
    instance_ = new SettingsManager();
    instance_->init();
}

std::shared_ptr<PluginHelper> SettingsManager::lookup_plugin(const std::string& id)
{
    auto iter = this->plugins_.find(id);
    if (iter != this->plugins_.end())
    {
        return iter->second;
    }
    return nullptr;
}

void SettingsManager::deactivate_plugins()
{
    for (auto& iter : this->plugins_)
    {
        CONTINUE_IF_TRUE(!iter.second->activate());

        CCErrorCode error_code = CCErrorCode::SUCCESS;
        iter.second->deactivate_plugin(error_code);

        if (error_code != CCErrorCode::SUCCESS)
        {
            KLOG_WARNING("%s", CC_ERROR2STR(error_code).c_str());
        }
    }
}

void SettingsManager::GetPlugins(MethodInvocation& invocation)
{
    std::vector<Glib::ustring> ids;
    for (auto iter = this->plugins_.begin(); iter != this->plugins_.end(); ++iter)
    {
        ids.push_back(iter->first);
    }
    invocation.ret(ids);
}

void SettingsManager::GetActivatedPlugins(MethodInvocation& invocation)
{
    std::vector<Glib::ustring> ids;
    for (auto iter = this->plugins_.begin(); iter != this->plugins_.end(); ++iter)
    {
        if (iter->second && iter->second->activate())
        {
            ids.push_back(iter->first);
        }
    }
    invocation.ret(ids);
}

void SettingsManager::ActivatePlugin(const Glib::ustring& id, MethodInvocation& invocation)
{
    KLOG_DEBUG("Active plugin by plugin id,the plugin id is %s.", id.c_str());
    auto plugin = this->lookup_plugin(id);
    if (!plugin)
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_PLUGIN_NOT_EXIST_1);
    }

    CCErrorCode error_code = CCErrorCode::SUCCESS;
    if (!plugin->activate_plugin(error_code))
    {
        DBUS_ERROR_REPLY_AND_RET(error_code);
    }

    invocation.ret();
}

void SettingsManager::DeactivatePlugin(const Glib::ustring& id, MethodInvocation& invocation)
{
    KLOG_DEBUG("Deactivate plugin by plugin id,the plugin id is %s.", id.c_str());
    auto plugin = this->lookup_plugin(id);
    if (!plugin)
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_PLUGIN_NOT_EXIST_2);
    }

    CCErrorCode error_code = CCErrorCode::SUCCESS;
    if (!plugin->deactivate_plugin(error_code))
    {
        DBUS_ERROR_REPLY_AND_RET(error_code);
    }

    invocation.ret();
}

void SettingsManager::init()
{
    KLOG_DEBUG("Init Settings Manager.");

    if (!g_module_supported())
    {
        KLOG_WARNING("Kiran-session-daemon is not able to initialize the plugins.");
        return;
    }

    // load  plugins
    auto plugin_config_path = Glib::build_filename(KCC_PLUGIN_DIR, PLUGIN_CONFIG_NAME);
    this->load_plugins(plugin_config_path);

    // connect and regist dbus
    this->dbus_init();
}

void SettingsManager::load_plugins(const std::string& file_name)
{
    Glib::KeyFile plugins_file;

    try
    {
        plugins_file.load_from_file(file_name);

        for (const auto& group : plugins_file.get_groups())
        {
            PluginInfo info;
            info.id = group;
            info.name = plugins_file.get_string(group, "Name");
            info.description = plugins_file.get_string(group, "Description");
            auto available = plugins_file.get_boolean(group, "Available");
            if (available)
            {
                auto plugin_helper = std::make_shared<PluginHelper>(info);
                CCErrorCode error_code = CCErrorCode::SUCCESS;
                if (!plugin_helper->activate_plugin(error_code))
                {
                    KLOG_WARNING("Failed to load %s plugin.", info.name.c_str());
                }
                else
                {
                    this->plugins_.emplace(info.id, plugin_helper);
                }
            }
        }
    }
    catch (const Glib::Error& e)
    {
        KLOG_WARNING("Failed to load plugins: %s.", e.what().c_str());
    }
}

void SettingsManager::dbus_init()
{
#ifdef KCC_SYSTEM_TYPE
    this->dbus_connect_id_ = Gio::DBus::own_name(Gio::DBus::BUS_TYPE_SYSTEM,
                                                 CC_DAEMON_DBUS_NAME,
                                                 sigc::mem_fun(this, &SettingsManager::on_bus_acquired),
                                                 sigc::mem_fun(this, &SettingsManager::on_name_acquired),
                                                 sigc::mem_fun(this, &SettingsManager::on_name_lost));
#elif KCC_SESSION_TYPE

    this->dbus_connect_id_ = Gio::DBus::own_name(Gio::DBus::BUS_TYPE_SESSION,
                                                 CC_DAEMON_DBUS_NAME,
                                                 sigc::mem_fun(this, &SettingsManager::on_bus_acquired),
                                                 sigc::mem_fun(this, &SettingsManager::on_name_acquired),
                                                 sigc::mem_fun(this, &SettingsManager::on_name_lost));
#endif
}  // namespace Kiran

void SettingsManager::on_bus_acquired(const Glib::RefPtr<Gio::DBus::Connection>& connect,
                                      Glib::ustring name)
{
    if (!connect)
    {
        KLOG_WARNING("Failed to connect dbus with %s", name.c_str());
        return;
    }
    try
    {
        this->object_register_id_ = this->register_object(connect, CC_DAEMON_OBJECT_PATH);
    }
    catch (const Glib::Error& e)
    {
        KLOG_WARNING("Register object_path %s fail: %s.", CC_DAEMON_OBJECT_PATH, e.what().c_str());
    }
}

void SettingsManager::on_name_acquired(const Glib::RefPtr<Gio::DBus::Connection>& connect,
                                       Glib::ustring name)
{
    KLOG_DEBUG("Success to register dbus name: %s", name.c_str());
}

void SettingsManager::on_name_lost(const Glib::RefPtr<Gio::DBus::Connection>& connect,
                                   Glib::ustring name)
{
    KLOG_WARNING("Failed to register dbus name: %s", name.c_str());
}

}  // namespace Kiran