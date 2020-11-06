/*
 * @Author       : tangjie02
 * @Date         : 2020-07-24 13:42:19
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-11-06 17:53:59
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/plugins/accounts/accounts-util.cpp
 */
#include "plugins/accounts/accounts-util.h"

#include <fcntl.h>
#include <glib/gi18n.h>

enum CommandExitStatus
{
    // success
    COMMAND_EXIT_STATUS_SUCCESS = 0,
    // can't update password file
    COMMAND_EXIT_STATUS_PW_UPDATE = 1,
    // invalid command syntax
    COMMAND_EXIT_STATUS_USAGE = 2,
    // invalid argument to option
    COMMAND_EXIT_STATUS_BAD_ARG = 3,
    // UID already in use (and no -o)
    COMMAND_EXIT_STATUS_UID_IN_USE = 4,
    // passwd file contains errors
    COMMAND_EXIT_STATUS_BAD_PWFILE = 5,
    // specified usr/group doesn't exist
    COMMAND_EXIT_STATUS_NOTFOUND = 6,
    // user to modify is logged in
    COMMAND_EXIT_STATUS_USER_BUSY = 8,
    // username already in use
    COMMAND_EXIT_STATUS_NAME_IN_USE = 9,
    // can't update group file
    COMMAND_EXIT_STATUS_GRP_UPDATE = 10,
    // insufficient space to move home dir
    COMMAND_EXIT_STATUS_NOSPACE = 11,
    // can't create/remove/move home directory
    COMMAND_EXIT_STATUS_HOMEDIR = 12,
    // can't update SELinux user mapping
    COMMAND_EXIT_STATUS_SE_UPDATE_1 = 13,
    // can't update SELinux user mapping
    COMMAND_EXIT_STATUS_SE_UPDATE_2 = 14,
    // can't update the subordinate uid file
    COMMAND_EXIT_STATUS_SUB_UID_UPDATE = 16,
    // can't update the subordinate gid file
    COMMAND_EXIT_STATUS_SUB_GID_UPDATE = 18,

};

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
    if (write(fd, id.c_str(), id.length()) != (int)id.length())
    {
        LOG_WARNING("Failed to write loginuid '%s'\n", id.c_str());
    }
    close(fd);
}

bool AccountsUtil::spawn_with_login_uid(const Glib::RefPtr<Gio::DBus::MethodInvocation> invocation,
                                        const std::vector<std::string> argv,
                                        std::string &err)
{
    SETTINGS_PROFILE("command: %s.", StrUtils::join(argv, " ").c_str());

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
    LOG_DEBUG("status: %d.", status);
    return AccountsUtil::parse_exit_status(status, err);
}

bool AccountsUtil::parse_exit_status(int32_t exit_status, std::string &error)
{
    g_autoptr(GError) g_error = NULL;
    if (!WIFEXITED(exit_status))
    {
        auto result = g_spawn_check_exit_status(exit_status, &g_error);
        if (!result)
        {
            error = g_error->message;
        }
        return result;
    }
    switch (WEXITSTATUS(exit_status))
    {
    case COMMAND_EXIT_STATUS_SUCCESS:
        return true;
    case COMMAND_EXIT_STATUS_PW_UPDATE:
        error = _("Can't update password file");
        break;
    case COMMAND_EXIT_STATUS_USAGE:
        error = _("Invalid command syntax");
        break;
    case COMMAND_EXIT_STATUS_BAD_ARG:
        error = _("Invalid argument to option");
        break;
    case COMMAND_EXIT_STATUS_UID_IN_USE:
        error = _("UID already in use");
        break;
    case COMMAND_EXIT_STATUS_BAD_PWFILE:
        error = _("Passwd file contains errors");
        break;
    case COMMAND_EXIT_STATUS_NOTFOUND:
        error = _("Specified user/group doesn't exist");
        break;
    case COMMAND_EXIT_STATUS_USER_BUSY:
        error = _("User to modify is logged in");
        break;
    case COMMAND_EXIT_STATUS_NAME_IN_USE:
        error = _("Username already in use");
        break;
    case COMMAND_EXIT_STATUS_GRP_UPDATE:
        error = _("Can't update group file");
        break;
    case COMMAND_EXIT_STATUS_NOSPACE:
        error = _("Insufficient space to move home dir");
        break;
    case COMMAND_EXIT_STATUS_HOMEDIR:
        error = _("Can't create/remove/move home directory");
        break;
    case COMMAND_EXIT_STATUS_SE_UPDATE_1:
    case COMMAND_EXIT_STATUS_SE_UPDATE_2:
        error = _("Can't update SELinux user mapping");
        break;
    case COMMAND_EXIT_STATUS_SUB_UID_UPDATE:
        error = _("Can't update the subordinate uid file");
        break;
    case COMMAND_EXIT_STATUS_SUB_GID_UPDATE:
        error = _("Can't update the subordinate gid file");
        break;
    default:
        error = _("Unknown error");
        break;
    }
    return false;
}

}  // namespace Kiran
