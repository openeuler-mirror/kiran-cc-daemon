/*
 * @Author       : tangjie02
 * @Date         : 2020-06-18 14:28:54
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-09-04 16:16:20
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/src/plugin-helper.h
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

    bool activate_plugin(std::string &err);

    bool deactivate_plugin(std::string &err);

private:
    bool load_plugin_module(std::string &err);

private:
    PluginInfo plugin_info_;
    bool activate_;

    std::shared_ptr<Glib::Module> module_;
    std::shared_ptr<Plugin> plugin_;
};
}  // namespace Kiran