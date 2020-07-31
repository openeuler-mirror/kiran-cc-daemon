/*
 * @Author       : tangjie02
 * @Date         : 2020-07-06 10:01:58
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-07-31 09:20:42
 * @Description  : 
 * @FilePath     : /kiran-system-daemon/plugins/timedate/timedate-manager.h
 */

#include <timedate_dbus_stub.h>

#include <functional>

#include "lib/auth-manager.h"

namespace Kiran
{
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
    // 调整硬件时钟设置，如果local为true则按本地时区进行存储，否则按UTC存储，adjust_system为true则将硬件时间同步到系统时间，否则将系统时间同步到硬件时间
    virtual void SetLocalRTC(bool local,
                             bool adjust_system,
                             MethodInvocation &invocation);

    // 是否开启网络时间同步
    virtual void SetNTP(bool active,
                        MethodInvocation &invocation);

    /* Handle the setting of a property
     * This method will be called as a result of a call to <PropName>_set
     * and should implement the actual setting of the property value.
     * Should return true on success and false otherwise.
     */
    virtual bool Timezone_setHandler(const Glib::ustring &value) { return true; };
    virtual Glib::ustring Timezone_get();

    /* Handle the setting of a property
     * This method will be called as a result of a call to <PropName>_set
     * and should implement the actual setting of the property value.
     * Should return true on success and false otherwise.
     */
    virtual bool LocalRTC_setHandler(bool value) { return true; };
    virtual bool LocalRTC_get();

    /* Handle the setting of a property
     * This method will be called as a result of a call to <PropName>_set
     * and should implement the actual setting of the property value.
     * Should return true on success and false otherwise.
     */
    virtual bool CanNTP_setHandler(bool value) { return true; };
    virtual bool CanNTP_get();

    /* Handle the setting of a property
     * This method will be called as a result of a call to <PropName>_set
     * and should implement the actual setting of the property value.
     * Should return true on success and false otherwise.
     */
    virtual bool NTP_setHandler(bool value) { return true; };
    virtual bool NTP_get();

    /* Handle the setting of a property
     * This method will be called as a result of a call to <PropName>_set
     * and should implement the actual setting of the property value.
     * Should return true on success and false otherwise.
     */
    virtual bool TimeUSec_setHandler(guint64 value) { return true; };
    virtual guint64 TimeUSec_get();

    /* Handle the setting of a property
     * This method will be called as a result of a call to <PropName>_set
     * and should implement the actual setting of the property value.
     * Should return true on success and false otherwise.
     */
    virtual bool RTCTimeUSec_setHandler(guint64 value) { return true; };
    virtual guint64 RTCTimeUSec_get();

private:
    void init();

    Glib::VariantContainerBase call_systemd(const std::string &method_name, const Glib::VariantContainerBase &parameters);
    bool call_systemd_noresult(const std::string &method_name, const Glib::VariantContainerBase &parameters);

    static void finish_hwclock_call(GPid pid, gint status, gpointer user_data);
    void start_hwclock_call(bool hctosys,
                            bool local,
                            bool utc,
                            Glib::RefPtr<Gio::DBus::MethodInvocation> invocation,
                            AuthManager::AuthCheckHandler handler);

    void read_ntp_units();
    bool is_ntp_active();
    uint64_t get_system_time(void);
    uint64_t get_rtc_time(void);

    void funish_set_time(MethodInvocation invocation, int64_t request_time, int64_t requested_time, bool relative);

    std::string get_timezone(void);
    void set_localtime_file_context(const std::string &path);
    void update_kernel_utc_offset();
    bool check_timezone_name(const std::string &name);
    void finish_set_timezone(MethodInvocation invocation, std::string time_zone);

    bool is_rtc_local(void);
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