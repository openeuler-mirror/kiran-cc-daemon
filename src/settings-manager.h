/*
 * @Author       : tangjie02
 * @Date         : 2020-05-29 15:55:54
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-06-30 20:24:23
 * @Description  : 
 * @FilePath     : /kiran-system-daemon/src/settings-manager.h
 */

#include <map>
#include <string>

#include "src/plugin-info.h"

namespace Kiran
{
class SettingsManager
{
public:
    SettingsManager();
    virtual ~SettingsManager();

    static SettingsManager* get_instance() { return instance_; };

    static void global_init();

    static void global_deinit() { delete instance_; };

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
};
}  // namespace Kiran
