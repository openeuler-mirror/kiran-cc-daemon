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