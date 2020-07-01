/*
 * @Author       : tangjie02
 * @Date         : 2020-05-29 15:54:30
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-06-30 20:25:02
 * @Description  : 
 * @FilePath     : /kiran-system-daemon/src/settings-manager.cpp
 */

#include "src/settings-manager.h"

#include <fmt/format.h>

#include "lib/log.h"

namespace Kiran
{
#define DEFAULT_SETTINGS_PREFIX "com.unikylin.Kiran.SessionDaemon"

#define DEFAULT_DBUS_NAME "com.unikylin.Kiran.SessionDaemon"

SettingsManager::SettingsManager() : dbus_connect_id_(0)
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
    load_dir(KSD_PLUGIN_DIR G_DIR_SEPARATOR_S);

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

            if (!Glib::str_has_suffix(name, KSD_PLUGIN_EXT))
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
    SETTINGS_PROFILE("Loading plugin: ", file_name.c_str());

    auto plugin_info = std::make_shared<PluginInfo>();

    RETURN_VAL_IF_FALSE(plugin_info->load_from_file(file_name, err), false);

    if (!plugin_info->is_available())
    {
        err = fmt::format("Plugin is unavailable");
        return false;
    }

    auto location = plugin_info->get_location();

    auto iter = this->plugins_.emplace(location, plugin_info);

    if (!iter.second)
    {
        err = fmt::format("the location of the plugin {0} is conflict.", plugin_info->get_name());
        return false;
    }

    RETURN_VAL_IF_FALSE(plugin_info->activate_plugin(err), false);

    return true;
}

void SettingsManager::dbus_init()
{
    this->dbus_connect_id_ = Gio::DBus::own_name(Gio::DBus::BUS_TYPE_SESSION,
                                                 DEFAULT_DBUS_NAME,
                                                 sigc::mem_fun(this, &SettingsManager::on_bus_acquired),
                                                 sigc::mem_fun(this, &SettingsManager::on_name_acquired),
                                                 sigc::mem_fun(this, &SettingsManager::on_name_lost));
}

void SettingsManager::on_bus_acquired(const Glib::RefPtr<Gio::DBus::Connection>& connect,
                                      Glib::ustring name)
{
    for (auto iter = this->plugins_.begin(); iter != this->plugins_.end(); ++iter)
    {
        auto plugin = iter->second->get_plugin();
        plugin->register_object(connect);
    }
}

void SettingsManager::on_name_acquired(const Glib::RefPtr<Gio::DBus::Connection>& connect,
                                       Glib::ustring name)
{
}

void SettingsManager::on_name_lost(const Glib::RefPtr<Gio::DBus::Connection>& connect,
                                   Glib::ustring name)
{
}

}  // namespace Kiran