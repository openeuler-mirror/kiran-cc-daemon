/*
 * @Author       : tangjie02
 * @Date         : 2020-09-07 11:25:31
 * @LastEditors  : tangjie02
 * @LastEditTime : 2096-10-15 23:34:11
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/plugins/display/display-output.h
 */

#pragma once

#include <monitor_dbus_stub.h>
//
#include "plugins/display/xrandr-manager.h"

namespace Kiran
{
struct MonitorInfo
{
    RROutput id;
    // output的名字，例如VGA-1, HDMI-1
    std::string name;
    // 显示接口是否有连接了显示设备
    bool connected;
    // 在屏幕中显示的位置
    int32_t x;
    int32_t y;
    // 分辨率大小
    uint32_t width;
    uint32_t height;
    // 旋转类型
    RotationType rotation;
    // 翻转类型
    ReflectType reflect;
    // 刷新率
    double refresh_rate;
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
    uint32_t width;
    uint32_t height;
    RotationType rotation;
    ReflectType reflect;
    double refresh_rate;
};

class DisplayMonitor : public SessionDaemon::Display::MonitorStub
{
public:
    DisplayMonitor(const MonitorInfo &monitor_info);
    virtual ~DisplayMonitor();

    void dbus_register();
    void dbus_unregister();

    void set(const MonitorBaseInfo &base);
    void get(MonitorBaseInfo &base);

    std::string generate_cmdline();

protected:
    virtual void SetMode(guint32 index, MethodInvocation &invocation);
    virtual void SetModeBySize(guint16 width, guint16 height, MethodInvocation &invocation);
    virtual void SetPosition(gint32 x, gint32 y, MethodInvocation &invocation);
    virtual void SetReflect(guint32 index, MethodInvocation &invocation);
    virtual void SetRotation(guint32 index, MethodInvocation &invocation);
    virtual void SetRefreshRate(double refresh_rate, MethodInvocation &invocation);

    virtual bool x_setHandler(gint32 value);
    virtual bool y_setHandler(gint32 value);
    virtual bool width_setHandler(guint32 value);
    virtual bool height_setHandler(guint32 value);
    virtual bool rotation_setHandler(guint16 value);
    virtual bool reflect_setHandler(guint16 value);
    virtual bool refresh_rate_setHandler(double value);
    virtual bool rotations_setHandler(const std::vector<guint16> &value);
    virtual bool reflects_setHandler(const std::vector<guint16> &value);
    virtual bool current_mode_setHandler(guint32 value);
    virtual bool modes_setHandler(const std::vector<guint32> &value);
    virtual bool npreferred_setHandler(gint32 value);

    virtual gint32 x_get() { return this->monitor_info_.x; };
    virtual gint32 y_get() { return this->monitor_info_.y; };
    virtual guint32 width_get() { return this->monitor_info_.width; };
    virtual guint32 height_get() { return this->monitor_info_.height; };
    virtual guint16 rotation_get() { return uint16_t(this->monitor_info_.rotation); };
    virtual guint16 reflect_get() { return uint16_t(this->monitor_info_.reflect); };
    virtual double refresh_rate_get() { return this->monitor_info_.refresh_rate; };
    virtual std::vector<guint16> rotations_get();
    virtual std::vector<guint16> reflects_get();
    virtual guint32 current_mode_get() { return this->monitor_info_.mode; };
    virtual std::vector<guint32> modes_get();
    virtual gint32 npreferred_get() { return this->monitor_info_.npreferred; };

private:
    Glib::RefPtr<Gio::DBus::Connection> dbus_connect_;
    uint32_t object_register_id_;
    Glib::DBusObjectPathString object_path_;

    MonitorInfo monitor_info_;
};
}  // namespace Kiran
