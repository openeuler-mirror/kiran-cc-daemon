/*
 * @Author       : tangjie02
 * @Date         : 2020-09-07 09:52:51
 * @LastEditors  : tangjie02
 * @LastEditTime : 2096-10-15 23:33:41
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/plugins/display/display-manager.h
 */
#pragma once

#include <display_dbus_stub.h>
//

#include "plugins/display/display-monitor.h"
#include "plugins/display/display.hxx"
#include "plugins/display/xrandr-manager.h"

namespace Kiran
{
#define DISPLAY_CONF_DIR "unikylin/kiran/session-daemon/display"

class DisplayManager : public SessionDaemon::DisplayStub
{
public:
    DisplayManager(XrandrManager* xrandr_manager);
    virtual ~DisplayManager();

    static DisplayManager* get_instance() { return instance_; };

    static void global_init(XrandrManager* xrandr_manager);

    static void global_deinit() { delete instance_; };

protected:
    // 应用之前的修改
    virtual void ApplyChanges(MethodInvocation& invocation);
    // 恢复之前的修改
    virtual void ResetChanges(MethodInvocation& invocation);
    // 将之前的修改保存到文件，保存之后无法再恢复到之前的修改状态
    virtual void Save(MethodInvocation& invocation);

    void ListOutputs(MethodInvocation& invocation);

private:
    void init();
    void load_monitors();
    bool load_display_config(std::string& err);
    bool apply_display_config(std::string& err);
    bool apply_screen_config(const ScreenConfigInfo& screen_config, std::string& err);

    std::shared_ptr<DisplayMonitor> get_monitor(const std::string& uuid);

    // 将uuid进行排序后拼接
    std::string get_monitors_id();
    std::string get_monitors_config_id(const ScreenConfigInfo::MonitorSequence& monitors);

    void on_bus_acquired(const Glib::RefPtr<Gio::DBus::Connection>& connect, Glib::ustring name);
    void on_name_acquired(const Glib::RefPtr<Gio::DBus::Connection>& connect, Glib::ustring name);
    void on_name_lost(const Glib::RefPtr<Gio::DBus::Connection>& connect, Glib::ustring name);

private:
    static DisplayManager* instance_;
    XrandrManager* xrandr_manager_;

    std::string display_file_path_;

    std::map<std::string, std::shared_ptr<DisplayMonitor>> monitors_;
    std::unique_ptr<Kiran::DisplayConfigInfo> display_config_;
    uint32_t dbus_connect_id_;
    uint32_t object_register_id_;
};
}  // namespace Kiran
