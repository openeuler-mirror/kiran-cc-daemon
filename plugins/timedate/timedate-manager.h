/*
 * @Author       : tangjie02
 * @Date         : 2020-07-06 10:01:58
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-09-29 16:49:42
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/plugins/timedate/timedate-manager.h
 */

#include <timedate_dbus_stub.h>

#include <functional>

#include "lib/dbus/dbus.h"
#include "plugins/timedate/timedate-format.h"

namespace Kiran
{
struct ZoneInfo
{
    std::string country_code;
    std::string coordinates;
    std::string tz;
};

class TimedateManager : public SystemDaemon::TimeDateStub
{
private:
    struct HWClockCall
    {
        Glib::RefPtr<Gio::DBus::MethodInvocation> invocation;
        AuthManager::AuthCheckHandler handler;
    };

public:
    TimedateManager();
    virtual ~TimedateManager();

    static TimedateManager *get_instance() { return instance_; };

    static void global_init();

    static void global_deinit() { delete instance_; };

protected:
    // 设置系统(软件)时间，如果relative为真，则设置相对时间，否则为绝对时间，时间单位为微妙
    virtual void SetTime(gint64 requested_time,
                         bool relative,
                         MethodInvocation &invocation);
    // 设置系统时区
    virtual void SetTimezone(const Glib::ustring &time_zone,
                             MethodInvocation &invocation);

    // 获取时区列表
    virtual void GetZoneList(MethodInvocation &invocation);

    // 调整硬件时钟设置，如果local为true则按本地时区进行存储，否则按UTC存储，adjust_system为true则将硬件时间同步到系统时间，否则将系统时间同步到硬件时间
    virtual void SetLocalRTC(bool local,
                             bool adjust_system,
                             MethodInvocation &invocation);

    // 是否开启网络时间同步
    virtual void SetNTP(bool active, MethodInvocation &invocation);

    // 获取可设置的日期时间格式列表，type参考TimedateFormatType
    virtual void GetDateFormatList(gint32 type, MethodInvocation &invocation);

    // 通过索引设置日期时间格式
    virtual void SetDateFormatByIndex(gint32 type, gint32 index, MethodInvocation &invocation);

    // 设置小时显示格式，format参考TimedateHourFormat
    virtual void SetHourFormat(gint32 format, MethodInvocation &invocation);

    // 开启时间显示到秒
    virtual void EnableSecondsShowing(bool enabled, MethodInvocation &invocation);

    virtual bool time_zone_setHandler(const Glib::ustring &value);
    virtual bool local_rtc_setHandler(bool value);
    virtual bool can_ntp_setHandler(bool value) { return true; };
    virtual bool ntp_setHandler(bool value);
    virtual bool system_time_setHandler(guint64 value) { return true; };
    virtual bool rtc_time_setHandler(guint64 value) { return true; };
    virtual bool date_long_format_index_setHandler(gint32 value);
    virtual bool date_short_format_index_setHandler(gint32 value);
    virtual bool hour_format_setHandler(gint32 value);
    virtual bool seconds_showing_setHandler(bool value);

    virtual Glib::ustring time_zone_get() { return this->time_zone_; };
    virtual bool local_rtc_get() { return this->local_rtc_; };
    virtual bool can_ntp_get() { return this->ntp_unit_name_.length() > 0; };
    virtual bool ntp_get() { return this->ntp_active_; };
    virtual guint64 system_time_get();
    virtual guint64 rtc_time_get();
    virtual gint32 date_long_format_index_get();
    virtual gint32 date_short_format_index_get();
    virtual gint32 hour_format_get();
    virtual bool seconds_showing_get();

private:
    void init();
    void init_ntp_units();
    // 获取可用的时间同步服务
    std::vector<std::string> get_ntp_units();
    // 开启NTP服务
    bool start_ntp_unit(const std::string &name, CCErrorCode &error_code);
    // 停止NTP服务
    bool stop_ntp_unit(const std::string &name, CCErrorCode &error_code);
    // NTP服务是否开启
    bool ntp_is_active();

    // 监控信号变化处理
    void ntp_unit_props_changed(const Gio::DBus::Proxy::MapChangedProperties &changed_properties, const std::vector<Glib::ustring> &invalidated_properties);
    void time_zone_changed(const Glib::RefPtr<Gio::File> &file, const Glib::RefPtr<Gio::File> &other_file, Gio::FileMonitorEvent event_type);
    void adjtime_changed(const Glib::RefPtr<Gio::File> &file, const Glib::RefPtr<Gio::File> &other_file, Gio::FileMonitorEvent event_type);
    void ntp_unit_changed(const Glib::RefPtr<Gio::File> &file, const Glib::RefPtr<Gio::File> &other_file, Gio::FileMonitorEvent event_type);

    std::string get_unit_object_path();

    std::vector<ZoneInfo> get_zone_infos();

    Glib::VariantContainerBase call_systemd(const std::string &method_name, const Glib::VariantContainerBase &parameters);
    bool call_systemd_noresult(const std::string &method_name, const Glib::VariantContainerBase &parameters);

    static void finish_hwclock_call(GPid pid, gint status, gpointer user_data);
    void start_hwclock_call(bool hctosys,
                            bool local,
                            bool utc,
                            Glib::RefPtr<Gio::DBus::MethodInvocation> invocation,
                            AuthManager::AuthCheckHandler handler);

    void funish_set_time(MethodInvocation invocation, int64_t request_time, int64_t requested_time, bool relative);

    void set_localtime_file_context(const std::string &path);
    void update_kernel_utc_offset();
    bool check_timezone_name(const std::string &name);
    void finish_set_timezone(MethodInvocation invocation, std::string time_zone);

    void finish_set_rtc_local_hwclock(MethodInvocation invocation, bool local);
    void finish_set_rtc_local(MethodInvocation invocation, bool local, bool adjust_system);

    void finish_set_ntp_active(MethodInvocation invocation, bool active);

    void on_bus_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name);
    void on_name_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name);
    void on_name_lost(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name);

private:
    static TimedateManager *instance_;

    const static std::vector<std::string> ntp_units_paths_;

    uint32_t dbus_connect_id_;

    uint32_t object_register_id_;

    Glib::RefPtr<Gio::DBus::Proxy> systemd_proxy_;
    Glib::RefPtr<Gio::DBus::Proxy> polkit_proxy_;

    std::string ntp_unit_name_;
    Glib::RefPtr<Gio::DBus::Proxy> ntp_unit_proxy_;

    Glib::RefPtr<Gio::FileMonitor> tz_monitor_;
    Glib::RefPtr<Gio::FileMonitor> adjtime_monitor_;
    std::vector<Glib::RefPtr<Gio::FileMonitor>> ntp_unit_monitors_;

    std::string time_zone_;
    bool local_rtc_;
    bool ntp_active_;

    TimedateFormat timedate_format_;
};
}  // namespace Kiran