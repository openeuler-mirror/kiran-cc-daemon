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
            KLOG_WARNING_ACCOUNTS("Failed to call GetConnectionUnixProcessID: %s", e.what().c_str());
            return false;
        }
    }
    else
    {
        KLOG_WARNING_ACCOUNTS("Failed to create dbus proxy for org.freedesktop.DBus");
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
            KLOG_WARNING_ACCOUNTS("Failed to call GetConnectionUnixUser: %s", e.what().c_str());
            return false;
        }
    }
    else
    {
        KLOG_WARNING_ACCOUNTS("Failed to create dbus proxy for org.freedesktop.DBus");
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
            KLOG_DEBUG_ACCOUNTS("%s", e.what().c_str());
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
        KLOG_WARNING_ACCOUNTS("Failed to write loginuid '%s'\n", id.c_str());
    }
    close(fd);
}

bool AccountsUtil::spawn_with_login_uid(const Glib::RefPtr<Gio::DBus::MethodInvocation> invocation,
                                        const std::vector<std::string> argv,
                                        std::string &error)
{
    std::string loginuid;
    std::string standard_error;
    int status;
    std::string working_directory;
    CCErrorCode error_code = CCErrorCode::SUCCESS;

    AccountsUtil::get_caller_loginuid(invocation, loginuid);

    try
    {
        Glib::spawn_sync(working_directory,
                         argv,
                         Glib::SPAWN_DEFAULT,
                         sigc::bind(&AccountsUtil::setup_loginuid, loginuid),
                         nullptr,
                         &standard_error,
                         &status);
    }
    catch (const Glib::SpawnError &e)
    {
        KLOG_WARNING_ACCOUNTS("%s.", e.what().c_str());
        error_code = CCErrorCode::ERROR_ACCOUNTS_SPAWN_SYNC_FAILED;
    }

    KLOG_INFO_ACCOUNTS("The result of command %s is %d.", StrUtils::join(argv, " ").c_str(), status);

    if (error_code == CCErrorCode::SUCCESS)
    {
        AccountsUtil::parse_exit_status(status, error_code);
    }

    if (error_code != CCErrorCode::SUCCESS)
    {
        error = CCError::get_error_desc(error_code, false);
        if (!standard_error.empty())
        {
            error += fmt::format(_(" ({0}, error code: 0x{1:x})"), StrUtils::rtrim(standard_error), int32_t(error_code));
        }
        else
        {
            error += fmt::format(_(" (error code: 0x{:x})"), int32_t(error_code));
        }
        return false;
    }
    return true;
}

bool AccountsUtil::parse_exit_status(int32_t exit_status, CCErrorCode &error_code)
{
    g_autoptr(GError) g_error = NULL;
    if (!WIFEXITED(exit_status))
    {
        auto result = g_spawn_check_exit_status(exit_status, &g_error);
        if (!result)
        {
            KLOG_WARNING_ACCOUNTS("%s.", g_error->message);
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
