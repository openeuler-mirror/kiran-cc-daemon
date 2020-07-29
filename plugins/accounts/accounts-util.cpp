/*
 * @Author       : tangjie02
 * @Date         : 2020-07-24 13:42:19
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-07-29 17:37:19
 * @Description  : 
 * @FilePath     : /kiran-system-daemon/plugins/accounts/accounts-util.cpp
 */
#include "plugins/accounts/accounts-util.h"

#include <fcntl.h>
#include <fmt/format.h>

#include "lib/log.h"
namespace Kiran
{
bool AccountsUtil::get_caller_pid(Glib::RefPtr<Gio::DBus::MethodInvocation> invocation, GPid &pid)
{
    guint32 pid_as_int;
    auto dbus_proxy = Gio::DBus::Proxy::create_sync(invocation->get_connection(),
                                                    "org.freedesktop.DBus",
                                                    "/org/freedesktop/DBus",
                                                    "org.freedesktop.DBus");

    if (dbus_proxy)
    {
        try
        {
            auto result = dbus_proxy->call_sync("GetConnectionUnixProcessID",
                                                Glib::VariantContainerBase(g_variant_new("(s)", invocation->get_sender().c_str())),
                                                -1);

            g_variant_get(result.gobj(), "(u)", &pid_as_int);
            pid = pid_as_int;
        }
        catch (const Glib::Error &e)
        {
            LOG_WARNING("failed to call GetConnectionUnixProcessID: %s", e.what().c_str());
            return false;
        }
    }
    else
    {
        LOG_WARNING("failed to create dbus proxy for org.freedesktop.DBus");
        return false;
    }

    return true;
}

bool AccountsUtil::get_caller_uid(Glib::RefPtr<Gio::DBus::MethodInvocation> invocation, int32_t &uid)
{
    auto dbus_proxy = Gio::DBus::Proxy::create_sync(invocation->get_connection(),
                                                    "org.freedesktop.DBus",
                                                    "/org/freedesktop/DBus",
                                                    "org.freedesktop.DBus");

    if (dbus_proxy)
    {
        try
        {
            auto result = dbus_proxy->call_sync("GetConnectionUnixUser",
                                                Glib::VariantContainerBase(g_variant_new("(s)", invocation->get_sender().c_str())),
                                                -1);

            g_variant_get(result.gobj(), "(u)", &uid);
        }
        catch (const Glib::Error &e)
        {
            LOG_WARNING("failed to call GetConnectionUnixUser: %s", e.what().c_str());
            return false;
        }
    }
    else
    {
        LOG_WARNING("failed to create dbus proxy for org.freedesktop.DBus");
        return false;
    }

    return true;
}

void AccountsUtil::get_caller_loginuid(const Glib::RefPtr<Gio::DBus::MethodInvocation> invocation, std::string &loginuid)
{
    GPid pid;
    int32_t uid;

    if (!AccountsUtil::get_caller_uid(invocation, uid))
    {
        uid = getuid();
    }

    if (AccountsUtil::get_caller_pid(invocation, pid))
    {
        auto path = fmt::format("/proc/{0}/loginuid", (int)pid);
        try
        {
            loginuid = Glib::file_get_contents(path);
        }
        catch (const Glib::FileError &e)
        {
            LOG_DEBUG("%s", e.what().c_str());
            loginuid = fmt::format("{0}", uid);
        }
    }
    else
    {
        loginuid = fmt::format("{0}", uid);
    }
}

void AccountsUtil::setup_loginuid(const std::string &id)
{
    auto fd = open("/proc/self/loginuid", O_WRONLY);
    write(fd, id.c_str(), id.length());
    close(fd);
}

bool AccountsUtil::spawn_with_login_uid(const Glib::RefPtr<Gio::DBus::MethodInvocation> invocation,
                                        const Glib::ArrayHandle<std::string> argv,
                                        std::string &err)
{
    std::string loginuid;
    int status;
    std::string working_directory;

    AccountsUtil::get_caller_loginuid(invocation, loginuid);

    try
    {
        Glib::spawn_sync(working_directory,
                         argv,
                         Glib::SPAWN_DEFAULT,
                         sigc::bind(&AccountsUtil::setup_loginuid, loginuid),
                         nullptr,
                         nullptr,
                         &status);
    }
    catch (const Glib::SpawnError &e)
    {
        err = e.what().raw();
        return false;
    }

    g_autoptr(GError) error = NULL;

    if (!g_spawn_check_exit_status(status, &error))
    {
        LOG_DEBUG("%s", error->message);
        return false;
    }
    return true;
}

Glib::RefPtr<Gio::FileMonitor> AccountsUtil::setup_monitor(const std::string &path,
                                                           const sigc::slot<void, const Glib::RefPtr<Gio::File> &, const Glib::RefPtr<Gio::File> &, Gio::FileMonitorEvent> &callback)
{
    auto file = Gio::File::create_for_path(path);
    try
    {
        auto monitor = file->monitor_file();
        monitor->signal_changed().connect(callback);
        return monitor;
    }
    catch (const Glib::Error &e)
    {
        LOG_WARNING("Unable to monitor %s: %s", path, e.what().c_str());
    }

    return Glib::RefPtr<Gio::FileMonitor>();
}

}  // namespace Kiran
