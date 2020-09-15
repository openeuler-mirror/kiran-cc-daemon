/*
 * @Author       : tangjie02
 * @Date         : 2020-09-07 09:52:51
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-09-15 15:54:58
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

// 显示模式
enum class DisplayMode : uint32_t
{
    // 所有显示器显示内容相同
    MIRRORS,
    // 按照横向扩展的方式显示
    EXTEND,
};

class DisplayManager : public SessionDaemon::DisplayStub
{
public:
    DisplayManager(XrandrManager* xrandr_manager);
    virtual ~DisplayManager();

    static DisplayManager* get_instance() { return instance_; };

    static void global_init(XrandrManager* xrandr_manager);

    static void global_deinit() { delete instance_; };

protected:
    // 获取所有monitor的object path
    virtual void ListMonitors(MethodInvocation& invocation);
    // 切换显示模式
    virtual void SwitchMode(guint32 mode, MethodInvocation& invocation);
    // 应用之前通过dbus调用做的修改
    virtual void ApplyChanges(MethodInvocation& invocation);
    // 恢复之前通过dbus调用做的修改
    virtual void ResetChanges(MethodInvocation& invocation);
    // 将之前的修改保存到文件，保存之后无法再恢复到之前的修改状态
    virtual void Save(MethodInvocation& invocation);

    virtual bool mode_setHandler(guint32 value);
    virtual guint32 mode_get() { return uint32_t(this->mode_); };

private:
    void init();
    // 加载settings内容
    void load_settings();
    // 每个已连接的output对应一个monitor对象
    void load_monitors();
    bool load_display_config(std::string& err);
    bool apply_display_config(std::string& err);
    bool apply_screen_config(const ScreenConfigInfo& screen_config, std::string& err);

    // 应用之前的修改
    bool apply(std::string& err);

    // 切换显示模式
    bool switch_mode(uint32_t mode, std::string& err);
    // 切换到镜像模式
    bool switch_to_mirrors(std::string& err);
    // 获取在所有monitor中都可用的mode列表
    ModeInfoVec monitors_common_modes(const DisplayMonitorVec& monitors);
    // 写还到扩展模式
    bool switch_to_extend(std::string& err);

    // 获取monitor
    std::shared_ptr<DisplayMonitor> get_monitor(uint32_t id);
    std::shared_ptr<DisplayMonitor> get_monitor_by_uid(const std::string& uid);
    // 目前所有monitor都是已连接的,未连接的暂时未创建monitor,这个逻辑先预留
    DisplayMonitorVec get_connected_monitors();

    // 将uid进行排序后拼接
    std::string get_monitors_uid();
    std::string get_c_monitors_uid(const ScreenConfigInfo::MonitorSequence& monitors);

    // 保存配置到文件
    bool save_to_file(std::string& err);

    // 处理xrandr变化的信号
    void resources_changed();

    void settings_changed(const Glib::ustring& key);

    void on_bus_acquired(const Glib::RefPtr<Gio::DBus::Connection>& connect, Glib::ustring name);
    void on_name_acquired(const Glib::RefPtr<Gio::DBus::Connection>& connect, Glib::ustring name);
    void on_name_lost(const Glib::RefPtr<Gio::DBus::Connection>& connect, Glib::ustring name);

private:
    static DisplayManager* instance_;
    XrandrManager* xrandr_manager_;

    // 显示配置文件路径
    std::string config_file_path_;
    // 显示配置内容
    std::unique_ptr<DisplayConfigInfo> display_config_;

    Glib::RefPtr<Gio::Settings> display_settings_;
    DisplayMode mode_;

    std::map<uint32_t, std::shared_ptr<DisplayMonitor>> monitors_;

    // dbus
    uint32_t dbus_connect_id_;
    uint32_t object_register_id_;
};
}  // namespace Kiran
