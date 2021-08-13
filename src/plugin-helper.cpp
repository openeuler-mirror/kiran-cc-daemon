/**
 * @Copyright (C) 2020 ~ 2021 KylinSec Co., Ltd. 
 *
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; If not, see <http: //www.gnu.org/licenses/>. 
 */

#include "src/plugin-helper.h"

namespace Kiran
{
#define PLUGIN_GROUP "Kiran Settings Plugin"

PluginHelper::PluginHelper(PluginInfo plugin_info) : plugin_info_(plugin_info),
                                                     activate_(false)
{
}

PluginHelper::~PluginHelper()
{
}

bool PluginHelper::activate_plugin(CCErrorCode &error_code)
{
    KLOG_PROFILE("");

    if (this->activate_)
    {
        KLOG_DEBUG("the plugin %s already is activate.", this->plugin_info_.name.c_str());
        return true;
    }

    if (!this->plugin_)
    {
        RETURN_VAL_IF_FALSE(load_plugin_module(error_code), false);
    }

    if (this->plugin_)
    {
        this->plugin_->activate();
        this->activate_ = true;
    }

    return true;
}

bool PluginHelper::deactivate_plugin(CCErrorCode &error_code)
{
    KLOG_PROFILE("");

    if (!this->activate_ || !this->plugin_)
    {
        KLOG_DEBUG("the plugin %s already is deactivate.", this->plugin_info_.name.c_str());
    }
    else
    {
        this->plugin_->deactivate();
        this->activate_ = false;
    }

    return true;
}

bool PluginHelper::load_plugin_module(CCErrorCode &error_code)
{
    KLOG_PROFILE("load module %s", this->plugin_info_.id.c_str());

    auto path = Glib::Module::build_path(KCC_PLUGIN_DIR, this->plugin_info_.id);
    g_return_val_if_fail(path.length() > 0, false);

    std::shared_ptr<Plugin> plugin2;

    this->module_ = std::make_shared<Glib::Module>(path);

    if (this->module_ && (*this->module_))
    {
        void *new_plugin_fun = nullptr;
        void *del_plugin_fun = nullptr;

        if (!this->module_->get_symbol("new_plugin", new_plugin_fun))
        {
            KLOG_WARNING("Not found function 'new_plugin' in module %s.", this->plugin_info_.id.c_str());
            error_code = CCErrorCode::ERROR_PLUGIN_NOTFOUND_NEW_PLUGIN_FUNC;
            return false;
        }

        if (!this->module_->get_symbol("delete_plugin", del_plugin_fun))
        {
            KLOG_WARNING("not found function 'delete_plugin' in module %s.", this->plugin_info_.id.c_str());
            error_code = CCErrorCode::ERROR_PLUGIN_NOTFOUND_DEL_PLUGIN_FUNC;
            return false;
        }

        this->plugin_ = std::shared_ptr<Plugin>((Kiran::Plugin *)((NewPluginFun)new_plugin_fun)(), (DelPluginFun)del_plugin_fun);
        return true;
    }
    else
    {
        error_code = CCErrorCode::ERROR_PLUGIN_OPEN_MODULE_FAILED;
        KLOG_WARNING("open module %s fail: %s.",
                     this->plugin_info_.id.c_str(),
                     this->module_ ? this->module_->get_last_error().c_str() : "unknown");
        return false;
    }

    return true;
}
}  // namespace Kiran