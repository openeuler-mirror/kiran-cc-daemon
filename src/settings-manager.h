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
