/*
 * @Author       : tangjie02
 * @Date         : 2020-09-07 11:25:31
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-09-15 15:41:26
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/plugins/display/display-monitor.h
 */

#pragma once

#include <monitor_dbus_stub.h>
//
#include "plugins/display/xrandr-manager.h"

namespace Kiran
{
#define DISPLAY_MONITOR_OBJECT_PATH "/com/unikylin/Kiran/SessionDaemon/Display/Monitor"

struct MonitorInfo
{
    // id
    RROutput id;
    // uid
    std::string uid;
    // output的名字，例如VGA-1, HDMI-1
    std::string name;
    // 显示接口是否有连接了显示设备
    bool connected;
    // 显示设备是否开启
    bool enabled;
    // 在屏幕中显示的位置
    int32_t x;
    int32_t y;
    // 旋转类型
    RotationType rotation;
    // 翻转类型
    ReflectType reflect;
    // 可设置的旋转列表
    RotationTypeVec rotations;
    // 可设置的翻转列表
    ReflectTypeVec reflects;
    // 当前使用的mode
    RRMode mode;
    // 可以使用的modes列表
    std::vector<RRMode> modes;
    // 最佳的modes列表
    int npreferred;
};

struct MonitorBaseInfo
{
    int32_t x;
    int32_t y;
    RotationType rotation;
    ReflectType reflect;
    RRMode mode;
};

class DisplayMonitor : public SessionDaemon::Display::MonitorStub
{
public:
    DisplayMonitor(const MonitorInfo &monitor_info);
    virtual ~DisplayMonitor();

    void update(const MonitorInfo &monitor_info);

    void dbus_register();
    void dbus_unregister();

    // 获取uid
    const std::string &get_uid() { return this->monitor_info_.uid; };
    // 获取object_path
    std::string get_object_path() { return this->object_path_; };
    // 获取最佳的mode,一般时可用mode列表的第一个
    std::shared_ptr<ModeInfo> get_best_mode();

    void set(const MonitorBaseInfo &base);
    void get(MonitorBaseInfo &base);

    // 生成设置monitor生效的命令参数，参数传递给xrandr命令执行
    std::string generate_cmdline();

    // 通过大小获取可用的mode列表
    ModeInfoVec get_modes_by_size(uint32_t width, uint32_t height);
    // 找到分辨率相同的mode列表，并匹配刷新率最接近的mode
    std::shared_ptr<ModeInfo> match_best_mode(uint32_t width, uint32_t height, double refresh_rate);

public:
    virtual guint32 id_get() { return this->monitor_info_.id; }
    virtual Glib::ustring name_get() { return this->monitor_info_.name; };
    virtual bool enabled_get() { return this->monitor_info_.enabled; }
    virtual bool connected_get() { return this->monitor_info_.connected; }
    virtual gint32 x_get() { return this->monitor_info_.x; };
    virtual gint32 y_get() { return this->monitor_info_.y; };
    virtual guint16 rotation_get() { return uint16_t(this->monitor_info_.rotation); };
    virtual guint16 reflect_get() { return uint16_t(this->monitor_info_.reflect); };
    virtual std::vector<guint16> rotations_get();
    virtual std::vector<guint16> reflects_get();
    virtual guint32 current_mode_get() { return this->monitor_info_.mode; };
    virtual std::vector<guint32> modes_get();
    virtual gint32 npreferred_get() { return this->monitor_info_.npreferred; };

protected:
    virtual void Enable(bool enabled, MethodInvocation &invocation);
    // 获取当前monitor可用的mode列表，列表元素包括ID，分辨率width，分辨率height和刷新率。
    virtual void ListModes(MethodInvocation &invocation);
    // 获取当前monitor最佳的mode列表，一般情况下只有一个元素，列表元素包括ID，分辨率width，分辨率height和刷新率。
    virtual void ListPreferredModes(MethodInvocation &invocation);
    // 获取当前使用的mode
    virtual void GetCurrentMode(MethodInvocation &invocation);
    // 设置当前使用的mode，参数为可用的mode列表的下标
    virtual void SetMode(guint32 id, MethodInvocation &invocation);
    // 通过分辨率设置当前使用的mode
    virtual void SetModeBySize(guint32 width, guint32 height, MethodInvocation &invocation);
    // 设置monitor在屏幕中显示的位置
    virtual void SetPosition(gint32 x, gint32 y, MethodInvocation &invocation);
    // 设置旋转，参数为rotations列表的下标
    virtual void SetRotation(guint16 rotation, MethodInvocation &invocation);
    // 设置翻转，参数为reflects列表的下标
    virtual void SetReflect(guint16 reflect, MethodInvocation &invocation);

    virtual bool id_setHandler(guint32 value);
    virtual bool name_setHandler(const Glib::ustring &value);
    virtual bool connected_setHandler(bool value);
    virtual bool enabled_setHandler(bool value);
    virtual bool x_setHandler(gint32 value);
    virtual bool y_setHandler(gint32 value);
    virtual bool rotation_setHandler(guint16 value);
    virtual bool reflect_setHandler(guint16 value);
    virtual bool rotations_setHandler(const std::vector<guint16> &value);
    virtual bool reflects_setHandler(const std::vector<guint16> &value);
    virtual bool current_mode_setHandler(guint32 value);
    virtual bool modes_setHandler(const std::vector<guint32> &value);
    virtual bool npreferred_setHandler(gint32 value);

private:
    int32_t find_index_by_mode_id(uint32_t mode_id);
    int32_t find_index_by_rotation(RotationType rotation);
    int32_t find_index_by_reflect(ReflectType reflect);

private:
    Glib::RefPtr<Gio::DBus::Connection> dbus_connect_;
    uint32_t object_register_id_;
    Glib::DBusObjectPathString object_path_;

    MonitorInfo monitor_info_;
};

using DisplayMonitorVec = std::vector<std::shared_ptr<DisplayMonitor>>;
}  // namespace Kiran
