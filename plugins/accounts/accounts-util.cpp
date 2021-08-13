/**
 * @Copyright (C) 2020 ~ 2021 KylinSec Co., Ltd. 
 *
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; If not, see <http: //www.gnu.org/licenses/>. 
 */

#include "plugins/accounts/accounts-util.h"

#include <fcntl.h>

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
            KLOG_WARNING("failed to call GetConnectionUnixProcessID: %s", e.what().c_str());
            return false;
        }
    }
    else
    {
        KLOG_WARNING("failed to create dbus proxy for org.freedesktop.DBus");
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
            KLOG_WARNING("failed to call GetConnectionUnixUser: %s", e.what().c_str());
            return false;
        }
    }
    else
    {
        KLOG_WARNING("failed to create dbus proxy for org.freedesktop.DBus");
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
            KLOG_DEBUG("%s", e.what().c_str());
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
        KLOG_WARNING("Failed to write loginuid '%s'\n", id.c_str());
    }
    close(fd);
}

bool AccountsUtil::spawn_with_login_uid(const Glib::RefPtr<Gio::DBus::MethodInvocation> invocation,
                                        const std::vector<std::string> argv,
                                        CCErrorCode &error_code)
{
    KLOG_DEBUG("command: %s.", StrUtils::join(argv, " ").c_str());

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
        KLOG_WARNING("%s.", e.what().c_str());
        error_code = CCErrorCode::ERROR_ACCOUNTS_SPAWN_SYNC_FAILED;
        return false;
    }
    KLOG_DEBUG("status: %d.", status);
    return AccountsUtil::parse_exit_status(status, error_code);
}

bool AccountsUtil::parse_exit_status(int32_t exit_status, CCErrorCode &error_code)
{
    g_autoptr(GError) g_error = NULL;
    if (!WIFEXITED(exit_status))
    {
        auto result = g_spawn_check_exit_status(exit_status, &g_error);
        if (!result)
        {
            KLOG_WARNING("%s.", g_error->message);
            error_code = CCErrorCode::ERROR_ACCOUNTS_SPAWN_EXIT_STATUS;
        }
        return result;
    }
    switch (WEXITSTATUS(exit_status))
    {
    case COMMAND_EXIT_STATUS_SUCCESS:
        return true;
    case COMMAND_EXIT_STATUS_PW_UPDATE:
        error_code = CCErrorCode::ERROR_ACCOUNTS_USER_COMMAND_PW_UPDATE;
        break;
    case COMMAND_EXIT_STATUS_USAGE:
        error_code = CCErrorCode::ERROR_ACCOUNTS_USER_COMMAND_USAGE;
        break;
    case COMMAND_EXIT_STATUS_BAD_ARG:
        error_code = CCErrorCode::ERROR_ACCOUNTS_USER_COMMAND_BAD_ARG;
        break;
    case COMMAND_EXIT_STATUS_UID_IN_USE:
        error_code = CCErrorCode::ERROR_ACCOUNTS_USER_COMMAND_UID_IN_USE;
        break;
    case COMMAND_EXIT_STATUS_BAD_PWFILE:
        error_code = CCErrorCode::ERROR_ACCOUNTS_USER_COMMAND_BAD_PWFILE;
        break;
    case COMMAND_EXIT_STATUS_NOTFOUND:
        error_code = CCErrorCode::ERROR_ACCOUNTS_USER_COMMAND_NOTFOUND;
        break;
    case COMMAND_EXIT_STATUS_USER_BUSY:
        error_code = CCErrorCode::ERROR_ACCOUNTS_USER_COMMAND_USER_BUSY;
        break;
    case COMMAND_EXIT_STATUS_NAME_IN_USE:
        error_code = CCErrorCode::ERROR_ACCOUNTS_USER_COMMAND_NAME_IN_USE;
        break;
    case COMMAND_EXIT_STATUS_GRP_UPDATE:
        error_code = CCErrorCode::ERROR_ACCOUNTS_USER_COMMAND_GRP_UPDATE;
        break;
    case COMMAND_EXIT_STATUS_NOSPACE:
        error_code = CCErrorCode::ERROR_ACCOUNTS_USER_COMMAND_NOSPACE;
        break;
    case COMMAND_EXIT_STATUS_HOMEDIR:
        error_code = CCErrorCode::ERROR_ACCOUNTS_USER_COMMAND_HOMEDIR;
        break;
    case COMMAND_EXIT_STATUS_SE_UPDATE_1:
        error_code = CCErrorCode::ERROR_ACCOUNTS_USER_COMMAND_SE_UPDATE_1;
        break;
    case COMMAND_EXIT_STATUS_SE_UPDATE_2:
        error_code = CCErrorCode::ERROR_ACCOUNTS_USER_COMMAND_SE_UPDATE_2;
        break;
    case COMMAND_EXIT_STATUS_SUB_UID_UPDATE:
        error_code = CCErrorCode::ERROR_ACCOUNTS_USER_COMMAND_SUB_UID_UPDATE;
        break;
    case COMMAND_EXIT_STATUS_SUB_GID_UPDATE:
        error_code = CCErrorCode::ERROR_ACCOUNTS_USER_COMMAND_SUB_GID_UPDATE;
        break;
    default:
        error_code = CCErrorCode::ERROR_ACCOUNTS_USER_COMMAND_UNKNOWN;
        break;
    }
    return false;
}

}  // namespace Kiran
