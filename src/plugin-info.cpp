/*
 * @Author       : tangjie02
 * @Date         : 2020-06-18 14:29:02
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-09-02 14:55:27
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/src/plugin-info.cpp
 */

#include "src/plugin-info.h"

namespace Kiran
{
#define PLUGIN_GROUP "Kiran Settings Plugin"

PluginInfo::PluginInfo() : available_(false),
                           activate_(false)
{
}

PluginInfo::~PluginInfo()
{
}

bool PluginInfo::load_from_file(const std::string &file_name, std::string &err)
{
    SETTINGS_PROFILE("load file %s", file_name.c_str());

    Glib::KeyFile plugin_file;

    this->file_name_ = file_name;

    try
    {
        if (!plugin_file.load_from_file(file_name))
        {
            err = fmt::format("Bad plugin file: {0}", file_name);
            return false;
        }

        this->location_ = plugin_file.get_string(PLUGIN_GROUP, "Module");
        if (this->location_.empty())
        {
            err = fmt::format("Could not find 'Module' in {0}", file_name);
            return false;
        }

        this->name_ = plugin_file.get_string(PLUGIN_GROUP, "Name");
        if (this->name_.empty())
        {
            err = fmt::format("Could not find 'Name' in {0}", file_name);
            return false;
        }

        this->description_ = plugin_file.get_string(PLUGIN_GROUP, "Description");
        if (this->description_.empty())
        {
            err = fmt::format("Could not find 'Description' in {0}", file_name);
            return false;
        }

        this->available_ = plugin_file.get_boolean(PLUGIN_GROUP, "Available");
    }
    catch (const Glib::KeyFileError &e)
    {
        err = e.what();
        return false;
    }

    return true;
}

bool PluginInfo::activate_plugin(std::string &err)
{
    SETTINGS_PROFILE("");

    if (!this->available_)
    {
        err = fmt::format("Plugin %s is unavaliable.", this->name_.c_str());
        return false;
    }

    if (this->activate_)
    {
        LOG_DEBUG("the plugin %s already is activate.", this->name_.c_str());
        return true;
    }

    if (!this->plugin_)
    {
        RETURN_VAL_IF_FALSE(load_plugin_module(err), false);
    }

    if (this->plugin_)
    {
        this->plugin_->activate();
        this->activate_ = true;
    }

    return true;
}

bool PluginInfo::deactivate_plugin(std::string &err)
{
    SETTINGS_PROFILE("");

    if (!this->available_)
    {
        err = fmt::format("Plugin %s is unavaliable.", this->name_.c_str());
        return false;
    }

    if (!this->activate_ || !this->plugin_)
    {
        LOG_DEBUG("the plugin %s already is deactivate.", this->name_.c_str());
    }
    else
    {
        this->plugin_->deactivate();
        this->activate_ = false;
    }

    return true;
}

bool PluginInfo::load_plugin_module(std::string &err)
{
    SETTINGS_PROFILE("load module %s", this->location_.c_str());

    auto dirname = Glib::path_get_dirname(this->file_name_);
    g_return_val_if_fail(dirname.length() > 0, false);

    auto path = Glib::Module::build_path(dirname, this->location_);
    g_return_val_if_fail(path.length() > 0, false);

    std::shared_ptr<Plugin> plugin2;

    this->module_ = std::make_shared<Glib::Module>(path);

    if (this->module_ && (*this->module_))
    {
        void *new_plugin_fun = nullptr;
        void *del_plugin_fun = nullptr;

        if (!this->module_->get_symbol("new_plugin", new_plugin_fun))
        {
            err = fmt::format("not found function 'new_plugin' in module {0}.", this->location_);
            return false;
        }

        if (!this->module_->get_symbol("delete_plugin", del_plugin_fun))
        {
            err = fmt::format("not found function 'delete_plugin' in module {0}.", this->location_);
            return false;
        }

        this->plugin_ = std::shared_ptr<Plugin>((Kiran::Plugin *)((NewPluginFun)new_plugin_fun)(), (DelPluginFun)del_plugin_fun);
        return true;
    }
    else
    {
        err = fmt::format("open module {0} fail: {1}.", this->location_, this->module_ ? this->module_->get_last_error().c_str() : "unknown");
        return false;
    }

    return true;
}
}  // namespace Kiran