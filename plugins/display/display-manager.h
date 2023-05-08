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

#pragma once

#include <display_dbus_stub.h>

#define DISPLAY_NEW_INTERFACE
#include "display_i.h"
#include "plugins/display/display-monitor.h"
#include "plugins/display/display.hxx"
#include "plugins/display/xrandr-manager.h"

namespace Kiran
{
class DisplayManager : public SessionDaemon::DisplayStub
{
public:
    DisplayManager(XrandrManager* xrandr_manager);
    virtual ~DisplayManager();

    static DisplayManager* get_instance() { return instance_; };

    static void global_init(XrandrManager* xrandr_manager);

    static void global_deinit() { delete instance_; };

    // 目前所有monitor都是已连接的,未连接的暂时未创建DisplayMonitor
    DisplayMonitorVec get_connected_monitors();
    // 获取开启的显示器
    DisplayMonitorVec get_enabled_monitors();

protected:
    // 获取所有monitor的object path
    virtual void ListMonitors(MethodInvocation& invocation);
    // 切换显示模式
    virtual void SwitchStyle(guint32 style, MethodInvocation& invocation);
    // 设置默认显示模式，默认显示模式会在程序第一次启动或者有连接的显示设备删除和添加时进行启用。
    virtual void SetDefaultStyle(guint32 style, MethodInvocation& invocation);
    // 应用之前通过dbus调用做的修改
    virtual void ApplyChanges(MethodInvocation& invocation);
    // 恢复之前通过dbus调用做的修改
    virtual void RestoreChanges(MethodInvocation& invocation);
    // 设置主显示器
    virtual void SetPrimary(const Glib::ustring& name, MethodInvocation& invocation);
    // 将之前的修改保存到文件，保存之后无法再恢复到之前的修改状态
    virtual void Save(MethodInvocation& invocation);
    // 设置窗口缩放因子
    virtual void SetWindowScalingFactor(gint32 window_scaling_factor, MethodInvocation& invocation);

    virtual bool default_style_setHandler(guint32 value);
    virtual bool primary_setHandler(const Glib::ustring& value);
    virtual bool window_scaling_factor_setHandler(gint32 value);

    virtual guint32 default_style_get() { return uint32_t(this->default_style_); };
    virtual Glib::ustring primary_get() { return this->primary_; };
    virtual gint32 window_scaling_factor_get() { return this->window_scaling_factor_; };

private:
    void init();
    // 加载settings内容
    void load_settings();
    // 每个已连接的output对应一个monitor对象
    void load_monitors();
    // 加载配置文件
    void load_config();

    // 应用配置到monitors_中，这里先要根据monitor_ids进行匹配找到对应的ScreenConfigInfo，然后再调用apply_screen_config。
    bool apply_config(CCErrorCode& error_code);
    // 应用配置到monitors_中
    bool apply_screen_config(const ScreenConfigInfo& screen_config, CCErrorCode& error_code);
    // 从monitors_提取参数填充配置
    void fill_screen_config(ScreenConfigInfo& screen_config);

    // 保存配置
    bool save_config(CCErrorCode& error_code);

    // 让monitors_中的参数实际生效，执行xrandr命令
    bool apply(CCErrorCode& error_code);

    // 切换显示模式
    bool switch_style(DisplayStyle style, CCErrorCode& error_code);
    // 切换并保存显示模式
    bool switch_style_and_save(DisplayStyle style, CCErrorCode& error_code);
    // 切换到镜像模式
    bool switch_to_mirrors(CCErrorCode& error_code);
    // 获取在所有monitor中都可用的mode列表
    ModeInfoVec monitors_common_modes(const DisplayMonitorVec& monitors);
    // 切换到扩展模式
    void switch_to_extend();
    // 切换到自定义模式
    bool switch_to_custom(CCErrorCode& error_code);
    // 切换到自动模式
    void switch_to_auto();

    // 获取monitor
    std::shared_ptr<DisplayMonitor> get_monitor(uint32_t id);
    std::shared_ptr<DisplayMonitor> get_monitor_by_uid(const std::string& uid);
    std::shared_ptr<DisplayMonitor> get_monitor_by_name(const std::string& name);
    // 优先匹配uid，如果有多个uid匹配，则再匹配name
    std::shared_ptr<DisplayMonitor> match_best_monitor(const std::string& uid,
                                                       const std::string& name);

    // 将uid进行排序后拼接
    std::string get_monitors_uid();
    std::string get_c_monitors_uid(const ScreenConfigInfo::MonitorSequence& monitors);

    // 保存配置到文件
    bool save_to_file(CCErrorCode& error_code);

    // 处理xrandr变化的信号
    void resources_changed();

    void display_settings_changed(const Glib::ustring& key);
    void xsettings_settings_changed(const Glib::ustring& key);

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
    Glib::RefPtr<Gio::Settings> xsettings_settings_;
    DisplayStyle default_style_;
    // 主显示器名字
    std::string primary_;
    // 窗口缩放率
    int32_t window_scaling_factor_;

    std::map<uint32_t, std::shared_ptr<DisplayMonitor>> monitors_;

    // dbus
    uint32_t dbus_connect_id_;
    uint32_t object_register_id_;
};
}  // namespace Kiran
