/*
 * @Author       : tangjie02
 * @Date         : 2020-05-29 15:54:30
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-08-31 17:48:08
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/src/settings-manager.cpp
 */

#include "src/settings-manager.h"

#include <fmt/format.h>

#include "lib/log.h"

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

std::shared_ptr<PluginInfo> SettingsManager::lookup_plugin(const std::string& name)
{
    auto iter = this->plugins_.find(name);
    if (iter != this->plugins_.end())
    {
        return iter->second;
    }
    return nullptr;
}

void SettingsManager::GetPlugins(MethodInvocation& invocation)
{
    SETTINGS_PROFILE("");
    std::vector<Glib::ustring> names;
    for (auto iter = this->plugins_.begin(); iter != this->plugins_.end(); ++iter)
    {
        names.push_back(iter->first);
    }
    invocation.ret(names);
}

void SettingsManager::GetActivatedPlugins(MethodInvocation& invocation)
{
    SETTINGS_PROFILE("");
    std::vector<Glib::ustring> names;
    for (auto iter = this->plugins_.begin(); iter != this->plugins_.end(); ++iter)
    {
        if (iter->second && iter->second->activate())
        {
            names.push_back(iter->first);
        }
    }
    invocation.ret(names);
}

void SettingsManager::ActivatePlugin(const Glib::ustring& plugin_name, MethodInvocation& invocation)
{
    SETTINGS_PROFILE("plugin name: %s.", plugin_name.c_str());
    auto plugin = this->lookup_plugin(plugin_name);
    if (!plugin)
    {
        auto err_message = fmt::format("the plugin {0} is not exist.", plugin_name.raw());
        invocation.ret(Glib::Error(G_DBUS_ERROR, G_DBUS_ERROR_FAILED, err_message.c_str()));
        return;
    }

    std::string err;
    if (!plugin->activate_plugin(err))
    {
        invocation.ret(Glib::Error(G_DBUS_ERROR, G_DBUS_ERROR_FAILED, err.c_str()));
        return;
    }

    invocation.ret();
}

void SettingsManager::DeactivatePlugin(const Glib::ustring& plugin_name, MethodInvocation& invocation)
{
    SETTINGS_PROFILE("plugin name: %s.", plugin_name.c_str());
    auto plugin = this->lookup_plugin(plugin_name);
    if (!plugin)
    {
        auto err_message = fmt::format("the plugin {0} is not exist.", plugin_name.raw());
        invocation.ret(Glib::Error(G_DBUS_ERROR, G_DBUS_ERROR_FAILED, err_message.c_str()));
        return;
    }

    std::string err;
    if (!plugin->deactivate_plugin(err))
    {
        invocation.ret(Glib::Error(G_DBUS_ERROR, G_DBUS_ERROR_FAILED, err.c_str()));
        return;
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

    load_all();
}

void SettingsManager::load_all()
{
    SETTINGS_PROFILE("");

    // load system plugins
    load_dir(KCC_PLUGIN_DIR G_DIR_SEPARATOR_S);

    // connect and regist dbus
    dbus_init();
}

void SettingsManager::load_dir(const std::string& path)
{
    SETTINGS_PROFILE("Loading settings plugins from dir: %s", path.c_str());

    try
    {
        Glib::Dir d(path);
        std::string err;
        for (auto iter = d.begin(); iter != d.end(); ++iter)
        {
            auto name = *iter;

            if (!Glib::str_has_suffix(name, KCC_PLUGIN_EXT))
            {
                continue;
            }

            auto file_name = Glib::build_filename(path, name);
            if (!Glib::file_test(file_name, Glib::FILE_TEST_IS_REGULAR))
            {
                LOG_WARNING("the type of file %s isn't regular.", file_name.c_str());
                continue;
            }

            if (!load_file(file_name, err))
            {
                LOG_WARNING("load file %s fail: %s.", file_name.c_str(), err.c_str());
            }
        }
    }
    catch (const Glib::Exception& e)
    {
        LOG_WARNING("open dir %s error: %s", path.c_str(), e.what().c_str());
        return;
    }
}

bool SettingsManager::load_file(const std::string& file_name, std::string& err)
{
    SETTINGS_PROFILE("Loading plugin: %s", file_name.c_str());

    auto plugin_info = std::make_shared<PluginInfo>();

    RETURN_VAL_IF_FALSE(plugin_info->load_from_file(file_name, err), false);

    auto location = plugin_info->get_location();
    auto name = plugin_info->get_name();

    if (this->plugins_.find(name) != this->plugins_.end())
    {
        err = fmt::format("the name of the plugin {0} is conflict.", plugin_info->get_name());
        return false;
    }

    auto iter = std::find_if(this->plugins_.begin(), this->plugins_.end(), [&location](const std::pair<std::string, std::shared_ptr<PluginInfo>>& plugin) {
        return location == plugin.second->get_location();
    });

    if (iter != this->plugins_.end())
    {
        err = fmt::format("the location of the plugin {0} is conflict.", plugin_info->get_location());
        return false;
    }
    else
    {
        this->plugins_.emplace(name, plugin_info);
    }

    if (plugin_info->available())
    {
        return plugin_info->activate_plugin(err);
    }

    return true;
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