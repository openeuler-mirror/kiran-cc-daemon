/*
 * @Author       : tangjie02
 * @Date         : 2020-07-06 10:01:58
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-07-08 09:28:41
 * @Description  : 
 * @FilePath     : /kiran-system-daemon/plugins/timedate/timedate-manager.h
 */

#include <timedate_dbus_stub.h>

#include <functional>

namespace Kiran
{
class TimedateManager : public System::TimeDateStub
{
private:
    struct NtpUnit
    {
        std::string name;
        std::string sort_name;
    };

    using AuthCheckHandler = std::function<void(MethodInvocation &)>;

    struct AuthCheck
    {
        AuthCheck(MethodInvocation &inv) : invocation(inv){};
        Glib::RefPtr<Gio::Cancellable> cancellable;
        sigc::connection cancel_connection;
        std::string cancel_string;
        MethodInvocation invocation;
        AuthCheckHandler handler;
    };

    struct HWClockCall
    {
        std::shared_ptr<MethodInvocation> invocation;
        AuthCheckHandler handler;
    };

public:
    TimedateManager();
    virtual ~TimedateManager();

    static TimedateManager *get_instance() { return instance_; };

    static void global_init();

    static void global_deinit() { delete instance_; };

protected:
    virtual void SetTime(gint64 requested_time,
                         bool relative,
                         bool user_interaction,
                         MethodInvocation &invocation);
    virtual void SetTimezone(const Glib::ustring &time_zone,
                             bool user_interaction,
                             MethodInvocation &invocation);
    virtual void SetLocalRTC(bool local,
                             bool adjust_system,
                             bool user_interaction,
                             MethodInvocation &invocation);
    virtual void SetNTP(bool active,
                        bool user_interaction,
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
    virtual bool NTPSynchronized_setHandler(bool value) { return true; };
    virtual bool NTPSynchronized_get();

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

    void finish_auth_check(Glib::RefPtr<Gio::AsyncResult> &res, std::shared_ptr<AuthCheck> auth_check);
    bool cancel_auth_check(std::shared_ptr<AuthCheck> auth_check);
    void start_auth_check(const std::string &action, bool user_interaction, MethodInvocation &invocation, AuthCheckHandler handler);

    Glib::VariantContainerBase call_systemd(const std::string &method_name, const Glib::VariantContainerBase &parameters);
    bool call_systemd_noresult(const std::string &method_name, const Glib::VariantContainerBase &parameters);

    static void finish_hwclock_call(GPid pid, gint status, gpointer user_data);
    void start_hwclock_call(bool hctosys,
                            bool local,
                            bool utc,
                            std::shared_ptr<MethodInvocation> invocation,
                            AuthCheckHandler handler);

    void read_ntp_units();
    bool is_ntp_active();
    bool get_clock_synchronized(void);
    uint64_t get_system_time(void);
    uint64_t get_rtc_time(void);

    void funish_set_time(MethodInvocation &invocation, int64_t request_time, int64_t requested_time, bool relative);

    std::string get_timezone(void);
    void set_localtime_file_context(const std::string &path);
    void update_kernel_utc_offset();
    bool check_timezone_name(const std::string &name);
    void finish_set_timezone(MethodInvocation &invocation, std::string time_zone);

    bool is_rtc_local(void);
    void finish_set_rtc_local_hwclock(MethodInvocation &invocation, bool local);
    void finish_set_rtc_local(MethodInvocation &invocation, bool local, bool adjust_system);

    void finish_set_ntp_active(MethodInvocation &invocation, bool active);

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

    int32_t running_auth_checks_;
};
}  // namespace Kiran