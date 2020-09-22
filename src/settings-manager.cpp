/*
 * @Author       : tangjie02
 * @Date         : 2020-05-29 15:54:30
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-09-04 16:11:24
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/src/settings-manager.cpp
 */

#include "src/settings-manager.h"

#include "lib/base/base.h"

namespace Kiran
{
#ifdef KCC_SYSTEM_TYPE
#define CC_DAEMON_DBUS_NAME "com.unikylin.Kiran.SystemDaemon"
#define CC_DAEMON_OBJECT_PATH "/com/unikylin/Kiran/SystemDaemon"
#elif KCC_SESSION_TYPE
#define CC_DAEMON_DBUS_NAME "com.unikylin.Kiran.SessionDaemon"
#define CC_DAEMON_OBJECT_PATH "/com/unikylin/Kiran/SessionDaemon"
#else
#define CC_DAEMON_DBUS_NAME "com.unikylin.Kiran.CCDaemon"
#define CC_DAEMON_OBJECT_PATH "/com/unikylin/Kiran/CCDaemon"
#error need to define KCC_SYSTEM_TYPE or KCC_SESSION_TYPE
#endif

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

void SettingsManager::GetPlugins(MethodInvocation& invocation)
{
    SETTINGS_PROFILE("");
    std::vector<Glib::ustring> ids;
    for (auto iter = this->plugins_.begin(); iter != this->plugins_.end(); ++iter)
    {
        ids.push_back(iter->first);
    }
    invocation.ret(ids);
}

void SettingsManager::GetActivatedPlugins(MethodInvocation& invocation)
{
    SETTINGS_PROFILE("");
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
    SETTINGS_PROFILE("plugin id: %s.", id.c_str());
    auto plugin = this->lookup_plugin(id);
    if (!plugin)
    {
        DBUS_ERROR_REPLY_AND_RET(CCError::ERROR_FAILED, "the plugin {0} is not exist.", id.c_str());
    }

    std::string err;
    if (!plugin->activate_plugin(err))
    {
        DBUS_ERROR_REPLY_AND_RET(CCError::ERROR_FAILED, err.c_str());
    }

    invocation.ret();
}

void SettingsManager::DeactivatePlugin(const Glib::ustring& id, MethodInvocation& invocation)
{
    SETTINGS_PROFILE("plugin id: %s.", id.c_str());
    auto plugin = this->lookup_plugin(id);
    if (!plugin)
    {
        DBUS_ERROR_REPLY_AND_RET(CCError::ERROR_FAILED, "the plugin {0} is not exist.", id.c_str());
    }

    std::string err;
    if (!plugin->deactivate_plugin(err))
    {
        DBUS_ERROR_REPLY_AND_RET(CCError::ERROR_FAILED, err.c_str());
    }

    invocation.ret();
}

void SettingsManager::init()
{
    SETTINGS_PROFILE("");

    if (!g_module_supported())
    {
        LOG_WARNING("kiran-session-daemon is not able to initialize the plugins.");
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
    SETTINGS_PROFILE("file_name: %s.", file_name.c_str());

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
                std::string err;
                if (!plugin_helper->activate_plugin(err))
                {
                    LOG_WARNING("failed to load plugin: %s.", err.c_str());
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
        LOG_WARNING("failed to load plugins: %s.", e.what().c_str());
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
    SETTINGS_PROFILE("name: %s", name.c_str());
    if (!connect)
    {
        LOG_WARNING("failed to connect dbus. name: %s", name.c_str());
        return;
    }
    try
    {
        this->object_register_id_ = this->register_object(connect, CC_DAEMON_OBJECT_PATH);
    }
    catch (const Glib::Error& e)
    {
        LOG_WARNING("register object_path %s fail: %s.", CC_DAEMON_OBJECT_PATH, e.what().c_str());
    }
}

void SettingsManager::on_name_acquired(const Glib::RefPtr<Gio::DBus::Connection>& connect,
                                       Glib::ustring name)
{
    LOG_DEBUG("success to register dbus name: %s", name.c_str());
}

void SettingsManager::on_name_lost(const Glib::RefPtr<Gio::DBus::Connection>& connect,
                                   Glib::ustring name)
{
    LOG_WARNING("failed to register dbus name: %s", name.c_str());
}

}  // namespace Kiran