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

    // 取消激活所有插件
    void deactivate_plugins();

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
