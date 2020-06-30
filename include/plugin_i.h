/*
 * @Author       : tangjie02
 * @Date         : 2020-05-29 15:56:22
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-06-19 13:55:50
 * @Description  : 
 * @FilePath     : /kiran-session-daemon/include/plugin_i.h
 */

#include <giomm.h>

namespace Kiran
{
class Plugin
{
public:
    virtual void activate() = 0;

    virtual void register_object(const Glib::RefPtr<Gio::DBus::Connection> &connection) = 0;
};

using NewPluginFun = void *(*)(void);
using DelPluginFun = void (*)(void *);

}  // namespace Kiran

#if defined(WIN32) || defined(_WIN64)
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT
#endif

#define PLUGIN_EXPORT_FUNC_DEF(plugin_name)               \
    extern "C" DLLEXPORT void *new_plugin()               \
    {                                                     \
        return new Kiran::plugin_name();                  \
    }                                                     \
    extern "C" DLLEXPORT void delete_plugin(void *plugin) \
    {                                                     \
        delete (Kiran::plugin_name *)plugin;              \
    }
