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

#include "plugins/timedate/timedate-manager.h"

#include <fcntl.h>
#include <fmt/format.h>
#include <glib/gi18n.h>
#include <linux/rtc.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/timex.h>
#include <sys/types.h>
#include <unistd.h>

#include <algorithm>
#include <cinttypes>

#include "config.h"
#include "lib/base/base.h"
#include "plugins/timedate/timedate-def.h"
#include "plugins/timedate/timedate-util.h"

#include "timedate-i.h"

#ifdef HAVE_SELINUX
#include <selinux/selinux.h>
#endif

/* This may be missing in libc headers */
#ifndef ADJ_SETOFFSET
#define ADJ_SETOFFSET 0x0100
#endif

namespace Kiran
{
#define HWCLOCK_PATH "/sbin/hwclock"
#define RTC_DEVICE "/dev/rtc"

#define ZONE_TABLE "zone.tab"
#define ZONE_TABLE_MIN_COLUMN 3
#define LOCALTIME_TO_ZONEINFO_PATH ".."
#define MAX_TIMEZONE_LENGTH 256

#define UNIT_PROP_ACTIVE_STATE "ActiveState"

#define TIMEZONE_DOMAIN "kiran-cc-daemon-timezones"

#define POLKIT_ACTION_SET_TIME "com.kylinsec.kiran.system-daemon.timedate.set-time"
#define POLKIT_ACTION_SET_NTP_ACTIVE "com.kylinsec.kiran.system-daemon.timedate.set-ntp"
#define POLKIT_ACTION_SET_RTC_LOCAL "com.kylinsec.kiran.system-daemon.timedate.set-local-rtc"
#define POLKIT_ACTION_SET_TIMEZONE "com.kylinsec.kiran.system-daemon.timedate.set-timezone"
#define POLKIT_AUTH_CHECK_TIMEOUT 20

TimedateManager *TimedateManager::instance_ = nullptr;
const std::vector<std::string> TimedateManager::ntp_units_paths_ = {"/etc/systemd/ntp-units.d", "/usr/lib/systemd/ntp-units.d"};

TimedateManager::TimedateManager() : dbus_connect_id_(0),
                                     object_register_id_(0),
                                     local_rtc_(false),
                                     ntp_active_(false)
{
}

TimedateManager::~TimedateManager()
{
    if (this->dbus_connect_id_)
    {
        Gio::DBus::unown_name(this->dbus_connect_id_);
    }
}

void TimedateManager::global_init()
{
    instance_ = new TimedateManager();
    instance_->init();
}

void TimedateManager::SetTime(gint64 requested_time,
                              bool relative,
                              MethodInvocation &invocation)
{
    KLOG_DEBUG_TIMEDATE("Requested Time as %" PRId64 " Relative is %d", requested_time, relative);
    if (this->ntp_get())
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_TIMEDATE_NTP_IS_ACTIVE);
    }

    int64_t request_time = g_get_monotonic_time();

    AuthManager::get_instance()->start_auth_check(POLKIT_ACTION_SET_TIME,
                                                  TRUE,
                                                  invocation.getMessage(),
                                                  std::bind(&TimedateManager::funish_set_time, this, std::placeholders::_1, request_time, requested_time, relative));
}

void TimedateManager::SetTimezone(const Glib::ustring &time_zone,
                                  MethodInvocation &invocation)
{
    KLOG_DEBUG_TIMEDATE("Set timezone as %s.", time_zone.c_str());
    if (!check_timezone_name(time_zone))
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_TIMEDATE_TIMEZONE_INVALIDE);
    }

    auto current_timezone = this->time_zone_get();
    if (current_timezone == time_zone)
    {
        invocation.ret();
        return;
    }

    AuthManager::get_instance()->start_auth_check(POLKIT_ACTION_SET_TIMEZONE,
                                                  TRUE,
                                                  invocation.getMessage(),
                                                  std::bind(&TimedateManager::finish_set_timezone, this, std::placeholders::_1, time_zone));
}

void TimedateManager::GetZoneList(MethodInvocation &invocation)
{
    std::vector<std::tuple<Glib::ustring, Glib::ustring, int64_t>> result;

    auto zone_infos = this->get_zone_infos();
    for (auto &zone_info : zone_infos)
    {
        auto local_zone_name = dgettext(TIMEZONE_DOMAIN, zone_info.tz.c_str());
        auto gmt = TimedateUtil::get_gmt_offset(zone_info.tz);
        result.push_back(std::make_tuple(zone_info.tz, std::string(local_zone_name), gmt));
    }
    invocation.ret(result);
}

void TimedateManager::SetLocalRTC(bool local,
                                  bool adjust_system,
                                  MethodInvocation &invocation)
{
    if (local == this->local_rtc_get())
    {
        invocation.ret();
        return;
    }

    AuthManager::get_instance()->start_auth_check(POLKIT_ACTION_SET_RTC_LOCAL,
                                                  TRUE,
                                                  invocation.getMessage(),
                                                  std::bind(&TimedateManager::finish_set_rtc_local, this, std::placeholders::_1, local, adjust_system));
}

void TimedateManager::SetNTP(bool active,
                             MethodInvocation &invocation)
{
    if (active == this->ntp_get())
    {
        invocation.ret();
        return;
    }

    if (this->ntp_unit_name_.length() == 0)
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_TIMEDATE_NO_NTP_UNIT);
    }

    AuthManager::get_instance()->start_auth_check(POLKIT_ACTION_SET_NTP_ACTIVE,
                                                  TRUE,
                                                  invocation.getMessage(),
                                                  std::bind(&TimedateManager::finish_set_ntp_active, this, std::placeholders::_1, active));
}

void TimedateManager::GetDateFormatList(gint32 type, MethodInvocation &invocation)
{
    switch (type)
    {
    case TimedateDateFormatType::TIMEDATE_FORMAT_TYPE_LONG:
    {
        auto formats = this->timedate_format_.get_long_formats();
        invocation.ret(std::vector<Glib::ustring>(formats.begin(), formats.end()));
        break;
    }
    case TimedateDateFormatType::TIMEDATE_FORMAT_TYPE_SHORT:
    {
        auto formats = this->timedate_format_.get_short_formats();
        invocation.ret(std::vector<Glib::ustring>(formats.begin(), formats.end()));
        break;
    }
    default:
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_TIMEDATE_UNKNOWN_DATE_FORMAT_TYPE_1);
    }
    return;
}

void TimedateManager::SetDateFormatByIndex(gint32 type, gint32 index, MethodInvocation &invocation)
{
    bool result = false;
    switch (type)
    {
    case TimedateDateFormatType::TIMEDATE_FORMAT_TYPE_LONG:
        result = this->date_long_format_index_set(index);
        break;
    case TimedateDateFormatType::TIMEDATE_FORMAT_TYPE_SHORT:
        result = this->date_short_format_index_set(index);
        break;
    default:
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_TIMEDATE_UNKNOWN_DATE_FORMAT_TYPE_2);
    }

    if (result)
    {
        invocation.ret();
    }
    else
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_TIMEDATE_SET_DATE_FORMAT_FAILED);
    }
}

void TimedateManager::SetHourFormat(gint32 format, MethodInvocation &invocation)
{
    if (!this->hour_format_set(format))
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_TIMEDATE_SET_HOUR_FORMAT_FAILED);
    }
    else
    {
        invocation.ret();
    }
}

void TimedateManager::EnableSecondsShowing(bool enabled, MethodInvocation &invocation)
{
    if (!this->seconds_showing_set(enabled))
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_TIMEDATE_SET_SECONDS_SHOWING_FAILED);
    }
    else
    {
        invocation.ret();
    }
}

bool TimedateManager::time_zone_setHandler(const Glib::ustring &value)
{
    this->time_zone_ = value.raw();
    return true;
}

bool TimedateManager::local_rtc_setHandler(bool value)
{
    this->local_rtc_ = value;
    return true;
}

bool TimedateManager::ntp_setHandler(bool value)
{
    this->ntp_active_ = value;
    return true;
}

bool TimedateManager::date_long_format_index_setHandler(gint32 value)
{
    return this->timedate_format_.set_date_long_format(value);
}

bool TimedateManager::date_short_format_index_setHandler(gint32 value)
{
    return this->timedate_format_.set_date_short_format(value);
}

bool TimedateManager::hour_format_setHandler(gint32 value)
{
    RETURN_VAL_IF_TRUE(value < 0 || value >= TimedateHourFormat::TIMEDATE_HOUSR_FORMAT_LAST, false);
    return this->timedate_format_.set_hour_format(TimedateHourFormat(value));
}

bool TimedateManager::seconds_showing_setHandler(bool value)
{
    return this->timedate_format_.set_seconds_showing(value);
}

guint64 TimedateManager::system_time_get()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)tv.tv_sec * 1000000 + tv.tv_usec;
}

guint64 TimedateManager::rtc_time_get()
{
    struct rtc_time rtc;
    struct tm tm;
    time_t rtc_time = 0;
    int fd, r;

    fd = open(RTC_DEVICE, O_RDONLY);
    if (fd < 0)
    {
        return 0;
    }

    r = ioctl(fd, RTC_RD_TIME, &rtc);
    close(fd);
    if (r)
    {
        return 0;
    }

    tm.tm_sec = rtc.tm_sec;
    tm.tm_min = rtc.tm_min;
    tm.tm_hour = rtc.tm_hour;
    tm.tm_mday = rtc.tm_mday;
    tm.tm_mon = rtc.tm_mon;
    tm.tm_year = rtc.tm_year;
    tm.tm_isdst = 0;

    /* This is the raw time as if the RTC was in UTC */
    rtc_time = timegm(&tm);

    return (uint64_t)rtc_time * 1000000;
}

gint32 TimedateManager::date_long_format_index_get()
{
    return this->timedate_format_.get_date_long_format_index();
}

gint32 TimedateManager::date_short_format_index_get()
{
    return this->timedate_format_.get_date_short_format_index();
}

gint32 TimedateManager::hour_format_get()
{
    return this->timedate_format_.get_hour_format();
}

bool TimedateManager::seconds_showing_get()
{
    return this->timedate_format_.get_seconds_showing();
}

void TimedateManager::init()
{
    bindtextdomain(TIMEZONE_DOMAIN, KCC_LOCALEDIR);
    bind_textdomain_codeset(TIMEZONE_DOMAIN, "UTF-8");

    this->dbus_connect_id_ = Gio::DBus::own_name(Gio::DBus::BUS_TYPE_SYSTEM,
                                                 TIMEDATE_DBUS_NAME,
                                                 sigc::mem_fun(this, &TimedateManager::on_bus_acquired),
                                                 sigc::mem_fun(this, &TimedateManager::on_name_acquired),
                                                 sigc::mem_fun(this, &TimedateManager::on_name_lost));

    this->systemd_proxy_ = Gio::DBus::Proxy::create_for_bus_sync(Gio::DBus::BUS_TYPE_SYSTEM, SYSTEMD_NAME, SYSTEMD_PATH, SYSTEMD_MANAGER_INTERFACE);
    this->polkit_proxy_ = Gio::DBus::Proxy::create_for_bus_sync(Gio::DBus::BUS_TYPE_SYSTEM, POLKIT_NAME, POLKIT_PATH, POLKIT_INTERFACE);

    this->tz_monitor_ = FileUtils::make_monitor_file(LOCALTIME_PATH, sigc::mem_fun(this, &TimedateManager::time_zone_changed));
    this->adjtime_monitor_ = FileUtils::make_monitor_file(ADJTIME_PATH, sigc::mem_fun(this, &TimedateManager::adjtime_changed));
    for (const auto &unit_dir : this->ntp_units_paths_)
    {
        auto unit_monitor = FileUtils::make_monitor_directory(unit_dir, sigc::mem_fun(this, &TimedateManager::ntp_unit_changed));
        this->ntp_unit_monitors_.push_back(unit_monitor);
    }

    this->time_zone_ = TimedateUtil::get_timezone();
    this->local_rtc_ = TimedateUtil::is_local_rtc();
    this->init_ntp_units();
    this->ntp_active_ = this->ntp_is_active();

    this->timedate_format_.init();
}

void TimedateManager::init_ntp_units()
{
    auto ntp_units = this->get_ntp_units();
    CCErrorCode error_code = CCErrorCode::SUCCESS;

    this->ntp_unit_name_.clear();
    for (auto &ntp_unit : ntp_units)
    {
        if (ntp_unit == ntp_units.back())
        {
            this->ntp_unit_name_ = ntp_unit;
            continue;
        }

        if (!this->stop_ntp_unit(ntp_unit, error_code))
        {
            KLOG_WARNING_TIMEDATE("%s", CC_ERROR2STR(error_code).c_str());
        }
    }

    auto unit_object_path = this->get_unit_object_path();

    if (unit_object_path.length() > 0)
    {
        this->ntp_unit_proxy_ = Gio::DBus::Proxy::create_for_bus_sync(Gio::DBus::BUS_TYPE_SYSTEM, SYSTEMD_NAME, unit_object_path, SYSTEMD_UNIT_INTERFACE);

        if (this->ntp_unit_proxy_)
        {
            this->ntp_unit_proxy_->signal_properties_changed().connect(sigc::mem_fun(this, &TimedateManager::ntp_unit_props_changed));
        }
        else
        {
            KLOG_WARNING_TIMEDATE("Failed to create dbus proxy. Object path: %s.", unit_object_path.c_str());
        }
    }
}

std::vector<std::string> TimedateManager::get_ntp_units()
{
    std::vector<std::string> ntp_units;

    for (auto iter = this->ntp_units_paths_.begin(); iter != this->ntp_units_paths_.end(); ++iter)
    {
        try
        {
            auto &unit_dir = *iter;
            Glib::Dir dir(unit_dir);

            for (auto dir_iter = dir.begin(); dir_iter != dir.end(); ++dir_iter)
            {
                auto entry = *dir_iter;
                auto path = fmt::format("{0}/{1}", unit_dir, entry);
                std::string contents;

                try
                {
                    contents = Glib::file_get_contents(path);
                }
                catch (const Glib::FileError &e)
                {
                    KLOG_WARNING_TIMEDATE("Failed to get contents of the file %s: %s", path.c_str(), e.what().c_str());
                    continue;
                }

                auto lines = StrUtils::split_lines(contents);

                for (auto line_iter = lines.begin(); line_iter != lines.end(); ++line_iter)
                {
                    auto line = *line_iter;
                    if (line.length() == 0 || line.at(0) == '#')
                    {
                        KLOG_DEBUG_TIMEDATE("The line %s is ingored. Length: %d", line.c_str(), line.length());
                        continue;
                    }

                    if (!call_systemd_noresult("LoadUnit", Glib::VariantContainerBase(g_variant_new("(s)", line.c_str()), false)))
                    {
                        KLOG_DEBUG_TIMEDATE("Failed to LoadUnit: %s.", line.c_str());
                        continue;
                    }

                    KLOG_DEBUG_TIMEDATE("Insert ntp unit: %s %s.", line.c_str(), entry.c_str());
                    ntp_units.push_back(line);
                }
            }
        }
        catch (const Glib::FileError &e)
        {
            KLOG_DEBUG_TIMEDATE("%s", e.what().c_str());
        }
    }

    // Remove duplicates.
    auto iter = std::unique(ntp_units.begin(), ntp_units.end());
    ntp_units.erase(iter, ntp_units.end());
    return ntp_units;
}

bool TimedateManager::start_ntp_unit(const std::string &name, CCErrorCode &error_code)
{
    GVariantBuilder builder;
    g_variant_builder_init(&builder, G_VARIANT_TYPE("as"));

    if (!call_systemd_noresult("StartUnit", Glib::VariantContainerBase(g_variant_new("(ss)", name.c_str(), "replace"), false)))
    {
        error_code = CCErrorCode::ERROR_TIMEDATE_START_NTP_FAILED;
        return false;
    }
    else
    {
        g_variant_builder_add(&builder, "s", name.c_str());
        call_systemd_noresult("EnableUnitFiles", Glib::VariantContainerBase(g_variant_new("(asbb)", &builder, FALSE, TRUE), false));
        call_systemd_noresult("Reload", Glib::VariantContainerBase(g_variant_new("()"), false));
    }
    return true;
}

bool TimedateManager::stop_ntp_unit(const std::string &name, CCErrorCode &error_code)
{
    GVariantBuilder builder;
    g_variant_builder_init(&builder, G_VARIANT_TYPE("as"));

    if (!call_systemd_noresult("StopUnit", Glib::VariantContainerBase(g_variant_new("(ss)", name.c_str(), "replace"), false)))
    {
        error_code = CCErrorCode::ERROR_TIMEDATE_STOP_NTP_FAILED;
        return false;
    }
    else
    {
        g_variant_builder_add(&builder, "s", name.c_str());
        call_systemd_noresult("DisableUnitFiles", Glib::VariantContainerBase(g_variant_new("(asb)", &builder, FALSE), false));
        call_systemd_noresult("Reload", Glib::VariantContainerBase(g_variant_new("()"), false));
    }
    return true;
}

bool TimedateManager::ntp_is_active()
{
    RETURN_VAL_IF_FALSE(this->ntp_unit_proxy_, false);

    Glib::VariantBase state;
    this->ntp_unit_proxy_->get_cached_property(state, UNIT_PROP_ACTIVE_STATE);
    RETURN_VAL_IF_FALSE(state.gobj() != NULL, false);

    auto state_str = Glib::VariantBase::cast_dynamic<Glib::Variant<Glib::ustring>>(state).get();

    return (state_str == "active" || state_str == "activating");
}

void TimedateManager::ntp_unit_props_changed(const Gio::DBus::Proxy::MapChangedProperties &changed_properties,
                                             const std::vector<Glib::ustring> &invalidated_properties)
{
    auto iter = changed_properties.find(UNIT_PROP_ACTIVE_STATE);
    RETURN_IF_TRUE(iter == changed_properties.end());

    auto state_str = Glib::VariantBase::cast_dynamic<Glib::Variant<Glib::ustring>>(iter->second).get();

    if (state_str == "active" || state_str == "activating")
    {
        this->ntp_set(true);
    }
    else
    {
        this->ntp_set(false);
    }
}

void TimedateManager::time_zone_changed(const Glib::RefPtr<Gio::File> &file,
                                        const Glib::RefPtr<Gio::File> &other_file,
                                        Gio::FileMonitorEvent event_type)
{
    RETURN_IF_TRUE(event_type != Gio::FILE_MONITOR_EVENT_CHANGED &&
                   event_type != Gio::FILE_MONITOR_EVENT_CREATED &&
                   event_type != Gio::FILE_MONITOR_EVENT_DELETED);

    auto tz = TimedateUtil::get_timezone();
    this->time_zone_set(tz);
}

void TimedateManager::adjtime_changed(const Glib::RefPtr<Gio::File> &file,
                                      const Glib::RefPtr<Gio::File> &other_file,
                                      Gio::FileMonitorEvent event_type)
{
    RETURN_IF_TRUE(event_type != Gio::FILE_MONITOR_EVENT_CHANGED &&
                   event_type != Gio::FILE_MONITOR_EVENT_CREATED &&
                   event_type != Gio::FILE_MONITOR_EVENT_DELETED);

    this->local_rtc_set(TimedateUtil::is_local_rtc());
}

void TimedateManager::ntp_unit_changed(const Glib::RefPtr<Gio::File> &file,
                                       const Glib::RefPtr<Gio::File> &other_file,
                                       Gio::FileMonitorEvent event_type)
{
    RETURN_IF_TRUE(event_type != Gio::FILE_MONITOR_EVENT_CHANGED &&
                   event_type != Gio::FILE_MONITOR_EVENT_CREATED &&
                   event_type != Gio::FILE_MONITOR_EVENT_DELETED);

    this->init_ntp_units();
}

std::string TimedateManager::get_unit_object_path()
{
    RETURN_VAL_IF_TRUE(this->ntp_unit_name_.size() <= 0, std::string());

    auto result = call_systemd("LoadUnit", Glib::VariantContainerBase(g_variant_new("(s)", this->ntp_unit_name_.c_str())));
    RETURN_VAL_IF_FALSE(result.gobj(), std::string());
    RETURN_VAL_IF_FALSE(result.get_n_children() > 0, std::string());

    auto path_base = result.get_child();
    auto object_path = Glib::VariantBase::cast_dynamic<Glib::Variant<Glib::ustring>>(path_base).get();
    return object_path;
}

std::vector<ZoneInfo> TimedateManager::get_zone_infos()
{
    std::vector<ZoneInfo> zone_infos;

    auto file_path = Glib::build_filename(ZONEINFO_PATH, ZONE_TABLE);

    std::string contents;
    try
    {
        contents = Glib::file_get_contents(file_path);
    }
    catch (const Glib::FileError &e)
    {
        KLOG_WARNING_TIMEDATE("Failed to get file contents: %s.", file_path.c_str());
        return zone_infos;
    }

    auto lines = StrUtils::split_lines(contents);

    for (auto &line : lines)
    {
        if (line[0] == '#' || line.length() == 0)
        {
            continue;
        }
        auto regex = Glib::Regex::create("\\s+");
        std::vector<std::string> tokens = regex->split(line);
        if (tokens.size() >= ZONE_TABLE_MIN_COLUMN)
        {
            ZoneInfo zone_info;
            zone_info.country_code = tokens[0];
            zone_info.coordinates = tokens[1];
            zone_info.tz = tokens[2];
            zone_infos.push_back(std::move(zone_info));
        }
        else
        {
            KLOG_WARNING_TIMEDATE("Ignore line: %s, the line is less than %d columns.", line.c_str(), ZONE_TABLE_MIN_COLUMN);
        }
    }
    return zone_infos;
}

Glib::VariantContainerBase TimedateManager::call_systemd(const std::string &method_name, const Glib::VariantContainerBase &parameters)
{
    KLOG_DEBUG_TIMEDATE("Call systemd method about %s.", method_name.c_str());
    Glib::VariantContainerBase retval;
    try
    {
        retval = this->systemd_proxy_->call_sync(method_name, parameters);
    }
    catch (const Glib::Error &e)
    {
        KLOG_WARNING_TIMEDATE("Failed to call systemd method: %s", e.what().c_str());
    }
    return retval;
}

bool TimedateManager::call_systemd_noresult(const std::string &method_name, const Glib::VariantContainerBase &parameters)
{
    KLOG_DEBUG_TIMEDATE("Call systemd noresult method about %s.", method_name.c_str());
    auto retval = call_systemd(method_name, parameters);
    if (retval.gobj())
    {
        return true;
    }
    return false;
}

void TimedateManager::finish_hwclock_call(GPid pid, gint status, gpointer user_data)
{
    auto hwclock_call = (HWClockCall *)user_data;
    GError *error = NULL;

    Glib::spawn_close_pid(pid);

    if (g_spawn_check_exit_status(status, &error))
    {
        if (hwclock_call->handler && hwclock_call->invocation)
            (hwclock_call->handler)(hwclock_call->invocation);
    }
    else
    {
        KLOG_WARNING_TIMEDATE("Hwclock failed: %s\n", error->message);
        if (hwclock_call->invocation)
        {
            auto err_message = fmt::format("hwclock failed: %s", error->message);
            hwclock_call->invocation->return_error(Glib::Error(G_DBUS_ERROR, G_DBUS_ERROR_FAILED, err_message.c_str()));
        }

        g_error_free(error);
    }

    delete hwclock_call;
}

void TimedateManager::start_hwclock_call(bool hctosys,
                                         bool local,
                                         bool utc,
                                         Glib::RefPtr<Gio::DBus::MethodInvocation> invocation,
                                         AuthManager::AuthCheckHandler handler)
{
    std::vector<std::string> argv;
    std::vector<std::string> envp;
    Glib::Pid pid;
    struct stat st;

    if (stat(RTC_DEVICE, &st) || !(st.st_mode & S_IFCHR))
    {
        if (invocation)
        {
            invocation->return_error(Glib::Error(G_DBUS_ERROR, G_DBUS_ERROR_FAILED, "No RTC device"));
        }
        return;
    }

    argv.push_back(HWCLOCK_PATH);
    argv.push_back("-f");
    argv.push_back(RTC_DEVICE);
    if (hctosys)
    {
        argv.push_back("--hctosys");
    }
    else
    {
        argv.push_back("--systohc");
    }

    if (local)
    {
        argv.push_back("--local");
    }

    if (utc)
    {
        argv.push_back("--utc");
    }

    try
    {
        Glib::spawn_async(std::string(),
                          argv,
                          envp,
                          Glib::SPAWN_DO_NOT_REAP_CHILD | Glib::SPAWN_STDOUT_TO_DEV_NULL | Glib::SPAWN_STDERR_TO_DEV_NULL,
                          Glib::SlotSpawnChildSetup(),
                          &pid);
    }
    catch (const Glib::SpawnError &e)
    {
        KLOG_WARNING_TIMEDATE("%s\n", e.what().c_str());
        if (invocation)
        {
            invocation->return_error(Glib::Error(G_DBUS_ERROR, G_DBUS_ERROR_FAILED, e.what().c_str()));
        }
        return;
    }

    auto hwclock_call = new HWClockCall();
    hwclock_call->invocation = invocation;
    hwclock_call->handler = handler;

    g_child_watch_add(pid, (GChildWatchFunc)(TimedateManager::finish_hwclock_call), hwclock_call);
}

void TimedateManager::funish_set_time(MethodInvocation invocation, int64_t request_time, int64_t requested_time, bool relative)
{
    struct timeval tv;
    struct timex tx;
    std::string err_message;

    if (relative)
    {
        tx.modes = ADJ_SETOFFSET | ADJ_NANO;

        tx.time.tv_sec = requested_time / 1000000;
        tx.time.tv_usec = requested_time - tx.time.tv_sec * 1000000;
        if (tx.time.tv_usec < 0)
        {
            tx.time.tv_sec--;
            tx.time.tv_usec += 1000000;
        }

        /* Convert to nanoseconds */
        tx.time.tv_usec *= 1000;

        if (adjtimex(&tx) < 0)
        {
            err_message = fmt::format("Failed to set system clock: {0}", strerror(errno));
        }
    }
    else
    {
        /* Compensate for the time taken by the authorization check */
        requested_time += g_get_monotonic_time() - request_time;

        tv.tv_sec = requested_time / 1000000;
        tv.tv_usec = requested_time - tv.tv_sec * 1000000;
        if (settimeofday(&tv, NULL))
        {
            err_message = fmt::format("Failed to set system clock: {0}", strerror(errno));
        }
    }

    if (err_message.length() > 0)
    {
        invocation.ret(Glib::Error(G_DBUS_ERROR, G_DBUS_ERROR_FAILED, err_message.c_str()));
    }
    else
    {
        invocation.ret();
        /* Set the RTC to the new system time */
        start_hwclock_call(false, false, false, Glib::RefPtr<Gio::DBus::MethodInvocation>(), nullptr);
    }
}

void TimedateManager::set_localtime_file_context(const std::string &path)
{
#ifdef HAVE_SELINUX
    security_context_t con;

    if (!is_selinux_enabled())
        return;

    if (matchpathcon_init_prefix(NULL, LOCALTIME_PATH))
        return;

    if (!matchpathcon(LOCALTIME_PATH, S_IFLNK, &con))
    {
        lsetfilecon(path.c_str(), con);
        freecon(con);
    }

    matchpathcon_fini();
#endif
}

void TimedateManager::update_kernel_utc_offset(void)
{
    struct timezone tz;
    struct timeval tv;
    struct tm *tm;

    bool updated = false;

    do
    {
        if (gettimeofday(&tv, &tz))
        {
            break;
        }

        tm = localtime(&tv.tv_sec);
        if (!tm)
        {
            break;
        }

        /* tm_gmtoff is in seconds east of UTC */
        tz.tz_minuteswest = -tm->tm_gmtoff / 60;
        if (settimeofday(NULL, &tz))
        {
            break;
        }

        updated = true;
    } while (0);

    if (!updated)
    {
        KLOG_WARNING_TIMEDATE("Failed to update kernel UTC offset");
    }
}

bool TimedateManager::check_timezone_name(const std::string &name)
{
    /* Check if the name is sane */
    if (name.length() == 0 ||
        name.front() == '/' ||
        strstr(name.c_str(), "//") ||
        strstr(name.c_str(), "..") ||
        name.length() > MAX_TIMEZONE_LENGTH)
        return false;

    auto iter = std::find_if(name.begin(), name.end(), [](char c) -> bool
                             { return !g_ascii_isalnum(c) && !strchr("+-_/", c); });

    if (iter != name.end())
    {
        return false;
    }

    /* Check if the correspoding file exists in the zoneinfo directory, it
           doesn't have to be listed in zone.tab */
    auto link = fmt::format("{0}{1}", ZONEINFO_PATH, name);
    struct stat st;
    if (stat(link.c_str(), &st) || !(st.st_mode & S_IFREG))
        return false;

    return true;
}

void TimedateManager::finish_set_timezone(MethodInvocation invocation, std::string time_zone)
{
    auto link = fmt::format("{0}{1}{2}", LOCALTIME_TO_ZONEINFO_PATH, ZONEINFO_PATH, time_zone);
    auto tmp = fmt::format("%s.%06u", LOCALTIME_PATH, g_random_int());
    bool successed = false;
    do
    {
        if (symlink(link.c_str(), tmp.c_str()))
            break;

        set_localtime_file_context(tmp);

        if (rename(tmp.c_str(), LOCALTIME_PATH))
        {
            unlink(tmp.c_str());
            break;
        }

        this->time_zone_set(time_zone);

        update_kernel_utc_offset();

        /* RTC in local needs to be set for the new timezone */
        if (this->local_rtc_get())
        {
            start_hwclock_call(false, false, false, Glib::RefPtr<Gio::DBus::MethodInvocation>(), nullptr);
        }
        successed = true;
    } while (0);

    if (!successed)
    {
        invocation.ret(Glib::Error(G_DBUS_ERROR, G_DBUS_ERROR_FAILED, "Failed to update " LOCALTIME_PATH));
    }
    else
    {
        invocation.ret();
    }
}

void TimedateManager::finish_set_rtc_local_hwclock(MethodInvocation invocation, bool local)
{
    this->local_rtc_set(local);
    invocation.ret();
}

void TimedateManager::finish_set_rtc_local(MethodInvocation invocation,
                                           bool local,
                                           bool adjust_system)
{
    start_hwclock_call(adjust_system,
                       local,
                       !local,
                       invocation.getMessage(),
                       std::bind(&TimedateManager::finish_set_rtc_local_hwclock, this, std::placeholders::_1, local));
}

void TimedateManager::finish_set_ntp_active(MethodInvocation invocation, bool active)
{
    CCErrorCode error_code = CCErrorCode::SUCCESS;
    bool result = false;
    if (active)
    {
        result = this->start_ntp_unit(this->ntp_unit_name_, error_code);
    }
    else
    {
        result = this->stop_ntp_unit(this->ntp_unit_name_, error_code);
    }

    if (!result)
    {
        DBUS_ERROR_REPLY_AND_RET(error_code);
    }

    this->ntp_set(active);
    invocation.ret();
}

void TimedateManager::on_bus_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name)
{
    if (!connect)
    {
        KLOG_WARNING_TIMEDATE("Failed to connect dbus with %s", name.c_str());
        return;
    }
    try
    {
        this->object_register_id_ = this->register_object(connect, TIMEDATE_OBJECT_PATH);
    }
    catch (const Glib::Error &e)
    {
        KLOG_WARNING_TIMEDATE("Register object_path %s fail: %s.", TIMEDATE_OBJECT_PATH, e.what().c_str());
    }
}

void TimedateManager::on_name_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name)
{
    KLOG_DEBUG_TIMEDATE("Success to register dbus name: %s", name.c_str());
}

void TimedateManager::on_name_lost(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name)
{
    KLOG_WARNING_TIMEDATE("Failed to register dbus name: %s", name.c_str());
}
}  // namespace Kiran