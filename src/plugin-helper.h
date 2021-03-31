/**
 * @file          /kiran-cc-daemon/src/plugin-helper.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#include <memory>
#include <string>

#include "lib/base/base.h"
#include "plugin_i.h"

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