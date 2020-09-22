/*
 * @Author       : tangjie02
 * @Date         : 2020-05-29 15:55:54
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-09-04 16:07:59
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/src/settings-manager.h
 */

#include <cc_daemon_dbus_stub.h>

#include <map>
#include <string>

#include "src/plugin-helper.h"

namespace Kiran
{
class SettingsManager : public CCDaemonStub
{
public:
    SettingsManager();
    virtual ~SettingsManager();

    static SettingsManager* get_instance() { return instance_; };

    static void global_init();

    static void global_deinit() { delete instance_; };

    std::shared_ptr<PluginHelper> lookup_plugin(const std::string& id);

protected:
    virtual void GetPlugins(MethodInvocation& invocation);

    virtual void GetActivatedPlugins(MethodInvocation& invocation);

    virtual void ActivatePlugin(const Glib::ustring& id, MethodInvocation& invocation);

    virtual void DeactivatePlugin(const Glib::ustring& id, MethodInvocation& invocation);

private:
    void init();

    void load_plugins(const std::string& file_name);

    void dbus_init();
    void on_bus_acquired(const Glib::RefPtr<Gio::DBus::Connection>& connect, Glib::ustring name);
    void on_name_acquired(const Glib::RefPtr<Gio::DBus::Connection>& connect, Glib::ustring name);
    void on_name_lost(const Glib::RefPtr<Gio::DBus::Connection>& connect, Glib::ustring name);

private:
    static SettingsManager* instance_;

    std::map<std::string, std::shared_ptr<PluginHelper>> plugins_;

    uint32_t dbus_connect_id_;

    uint32_t object_register_id_;
};
}  // namespace Kiran
