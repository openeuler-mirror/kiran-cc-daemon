/*
 * @Author       : tangjie02
 * @Date         : 2020-05-29 15:55:54
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-07-30 16:01:31
 * @Description  : 
 * @FilePath     : /kiran-system-daemon/src/settings-manager.h
 */

#include <system_daemon_dbus_stub.h>

#include <map>
#include <string>

#include "src/plugin-info.h"

namespace Kiran
{
class SettingsManager : public SystemDaemonStub
{
public:
    SettingsManager();
    virtual ~SettingsManager();

    static SettingsManager* get_instance() { return instance_; };

    static void global_init();

    static void global_deinit() { delete instance_; };

    std::shared_ptr<PluginInfo> lookup_plugin(const std::string& name);

protected:
    virtual void GetPlugins(MethodInvocation& invocation);

    virtual void GetActivatedPlugins(MethodInvocation& invocation);

    virtual void ActivatePlugin(const Glib::ustring& plugin_name, MethodInvocation& invocation);

    virtual void DeactivatePlugin(const Glib::ustring& plugin_name, MethodInvocation& invocation);

private:
    void init();

    void load_all();
    void load_dir(const std::string& path);
    bool load_file(const std::string& file_name, std::string& err);

    void dbus_init();
    void on_bus_acquired(const Glib::RefPtr<Gio::DBus::Connection>& connect, Glib::ustring name);
    void on_name_acquired(const Glib::RefPtr<Gio::DBus::Connection>& connect, Glib::ustring name);
    void on_name_lost(const Glib::RefPtr<Gio::DBus::Connection>& connect, Glib::ustring name);

private:
    static SettingsManager* instance_;

    std::map<std::string, std::shared_ptr<PluginInfo>> plugins_;

    uint32_t dbus_connect_id_;

    uint32_t object_register_id_;
};
}  // namespace Kiran
