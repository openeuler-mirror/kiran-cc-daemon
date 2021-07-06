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

#include <memory>
#include <string>

#include "lib/base/base.h"
#include "plugin-i.h"

namespace Kiran
{
struct PluginInfo
{
    std::string id;
    std::string name;
    std::string description;
};

class PluginHelper
{
public:
    PluginHelper(PluginInfo plugin_info);
    virtual ~PluginHelper();

    bool activate() { return this->activate_; }

    std::shared_ptr<Plugin> get_plugin() { return this->plugin_; }

    bool activate_plugin(CCErrorCode &error_code);

    bool deactivate_plugin(CCErrorCode &error_code);

private:
    bool load_plugin_module(CCErrorCode &error_code);

private:
    PluginInfo plugin_info_;
    bool activate_;

    std::shared_ptr<Glib::Module> module_;
    std::shared_ptr<Plugin> plugin_;
};
}  // namespace Kiran