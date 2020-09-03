/*
 * @Author       : tangjie02
 * @Date         : 2020-07-06 10:01:58
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-09-02 15:28:44
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/plugins/timedate/timedate-manager.h
 */

#include <timedate_dbus_stub.h>

#include <functional>

#include "lib/dbus/dbus.h"

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
    struct NtpUnit
    {
        std::string name;
        std::string sort_name;
    };

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

    virtual bool time_zone_setHandler(const Glib::ustring &value) { return true; };

    virtual bool local_rtc_setHandler(bool value) { return true; };
    virtual bool can_ntp_setHandler(bool value) { return true; };
    virtual bool ntp_setHandler(bool value) { return true; };
    virtual bool system_time_setHandler(guint64 value) { return true; };
    virtual bool rtc_time_setHandler(guint64 value) { return true; };

    virtual Glib::ustring time_zone_get();
    virtual bool local_rtc_get();
    virtual bool can_ntp_get();
    virtual bool ntp_get();
    virtual guint64 system_time_get();
    virtual guint64 rtc_time_get();

private:
    void init();

    std::vector<ZoneInfo> get_zone_infos();

    Glib::VariantContainerBase call_systemd(const std::string &method_name, const Glib::VariantContainerBase &parameters);
    bool call_systemd_noresult(const std::string &method_name, const Glib::VariantContainerBase &parameters);

    static void finish_hwclock_call(GPid pid, gint status, gpointer user_data);
    void start_hwclock_call(bool hctosys,
                            bool local,
                            bool utc,
                            Glib::RefPtr<Gio::DBus::MethodInvocation> invocation,
                            AuthManager::AuthCheckHandler handler);

    void read_ntp_units();
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

    std::vector<NtpUnit> ntp_units_;
};
}  // namespace Kiran