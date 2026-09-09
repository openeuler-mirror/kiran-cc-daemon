/**
 * Copyright (c) 2020 ~ 2025 KylinSec Co., Ltd.
 * kiran-cc-daemon is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     yuanxing <yuanxing@kylinsec.com.cn>
 */

#include "plugins/groups/groups-util.h"

#include <fcntl.h>
#include <glib/gi18n.h>
#include <unistd.h>

enum CommandExitStatus
{
    COMMAND_EXIT_STATUS_SUCCESS = 0,
    COMMAND_EXIT_STATUS_USAGE = 2,
    COMMAND_EXIT_STATUS_BAD_ARG = 3,
    COMMAND_EXIT_STATUS_GID_IN_USE = 4,
    COMMAND_EXIT_STATUS_NOTFOUND = 6,
    COMMAND_EXIT_STATUS_PRIMARY_GROUP = 8,
    COMMAND_EXIT_STATUS_NAME_IN_USE = 9,
    COMMAND_EXIT_STATUS_GRP_UPDATE = 10,
    COMMAND_EXIT_STATUS_CLEANUP_SERVICE = 11,
    COMMAND_EXIT_STATUS_PAM_USERNAME = 12,
    COMMAND_EXIT_STATUS_PAM_ERROR = 13,
};

namespace Kiran
{
bool GroupsUtil::get_caller_pid(Glib::RefPtr<Gio::DBus::MethodInvocation> invocation, GPid &pid)
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
            KLOG_WARNING_GROUPS("Failed to call GetConnectionUnixProcessID: %s", e.what().c_str());
            return false;
        }
    }
    else
    {
        KLOG_WARNING_GROUPS("Failed to create dbus proxy for org.freedesktop.DBus");
        return false;
    }

    return true;
}

bool GroupsUtil::get_caller_uid(Glib::RefPtr<Gio::DBus::MethodInvocation> invocation, int32_t &uid)
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
            KLOG_WARNING_GROUPS("Failed to call GetConnectionUnixUser: %s", e.what().c_str());
            return false;
        }
    }
    else
    {
        KLOG_WARNING_GROUPS("Failed to create dbus proxy for org.freedesktop.DBus");
        return false;
    }

    return true;
}

void GroupsUtil::get_caller_loginuid(const Glib::RefPtr<Gio::DBus::MethodInvocation> invocation, std::string &loginuid)
{
    GPid pid;
    int32_t uid;

    if (!GroupsUtil::get_caller_uid(invocation, uid))
    {
        uid = getuid();
    }

    if (GroupsUtil::get_caller_pid(invocation, pid))
    {
        auto path = fmt::format("/proc/{0}/loginuid", (int)pid);
        try
        {
            loginuid = Glib::file_get_contents(path);
        }
        catch (const Glib::FileError &e)
        {
            KLOG_DEBUG_GROUPS("%s", e.what().c_str());
            loginuid = fmt::format("{0}", uid);
        }
    }
    else
    {
        loginuid = fmt::format("{0}", uid);
    }
}

void GroupsUtil::setup_loginuid(const std::string &id)
{
    auto fd = open("/proc/self/loginuid", O_WRONLY);
    if (write(fd, id.c_str(), id.length()) != (int)id.length())
    {
        KLOG_WARNING_GROUPS("Failed to write loginuid '%s'\n", id.c_str());
    }
    close(fd);
}

bool GroupsUtil::spawn_with_login_uid(const Glib::RefPtr<Gio::DBus::MethodInvocation> invocation,
                                      const std::vector<std::string> argv,
                                      std::string &error)
{
    std::string loginuid;
    std::string standard_error;
    int status;
    CCErrorCode error_code = CCErrorCode::SUCCESS;

    GroupsUtil::get_caller_loginuid(invocation, loginuid);

    try
    {
        std::string working_directory;
        Glib::spawn_sync(working_directory,
                         argv,
                         Glib::SPAWN_DEFAULT,
                         sigc::bind(&GroupsUtil::setup_loginuid, loginuid),
                         nullptr,
                         &standard_error,
                         &status);
    }
    catch (const Glib::SpawnError &e)
    {
        KLOG_WARNING_GROUPS("%s.", e.what().c_str());
        error_code = CCErrorCode::ERROR_GROUPS_SPAWN_SYNC_FAILED;
    }

    KLOG_INFO_GROUPS("The result of command %s is %d.", StrUtils::join(argv, " ").c_str(), status);

    if (error_code == CCErrorCode::SUCCESS)
    {
        GroupsUtil::parse_exit_status(status, error_code);
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

bool GroupsUtil::parse_exit_status(int32_t exit_status, CCErrorCode &error_code)
{
    g_autoptr(GError) g_error = NULL;
    if (!WIFEXITED(exit_status))
    {
        auto result = g_spawn_check_exit_status(exit_status, &g_error);
        if (!result)
        {
            KLOG_WARNING_GROUPS("%s.", g_error->message);
            error_code = CCErrorCode::ERROR_GROUPS_SPAWN_EXIT_STATUS;
        }
        return result;
    }
    switch (WEXITSTATUS(exit_status))
    {
    case COMMAND_EXIT_STATUS_SUCCESS:
        return true;
    case COMMAND_EXIT_STATUS_USAGE:
        error_code = CCErrorCode::ERROR_GROUPS_GROUP_COMMAND_USAGE;
        break;
    case COMMAND_EXIT_STATUS_BAD_ARG:
        error_code = CCErrorCode::ERROR_GROUPS_GROUP_COMMAND_BAD_ARG;
        break;
    case COMMAND_EXIT_STATUS_GID_IN_USE:
        error_code = CCErrorCode::ERROR_GROUPS_GROUP_COMMAND_GID_IN_USE;
        break;
    case COMMAND_EXIT_STATUS_NOTFOUND:
        error_code = CCErrorCode::ERROR_GROUPS_GROUP_COMMAND_NOTFOUND;
        break;
    case COMMAND_EXIT_STATUS_PRIMARY_GROUP:
        error_code = CCErrorCode::ERROR_GROUPS_GROUP_COMMAND_PRIMARY_GROUP;
        break;
    case COMMAND_EXIT_STATUS_NAME_IN_USE:
        error_code = CCErrorCode::ERROR_GROUPS_GROUP_COMMAND_NAME_IN_USE;
        break;
    case COMMAND_EXIT_STATUS_GRP_UPDATE:
        error_code = CCErrorCode::ERROR_GROUPS_GROUP_COMMAND_GRP_UPDATE;
        break;
    case COMMAND_EXIT_STATUS_CLEANUP_SERVICE:
        error_code = CCErrorCode::ERROR_GROUPS_GROUP_COMMAND_CLEANUP_SERVICE;
        break;
    case COMMAND_EXIT_STATUS_PAM_USERNAME:
        error_code = CCErrorCode::ERROR_GROUPS_GROUP_COMMAND_PAM_USERNAME;
        break;
    case COMMAND_EXIT_STATUS_PAM_ERROR:
        error_code = CCErrorCode::ERROR_GROUPS_GROUP_COMMAND_PAM_ERROR;
        break;
    default:
        error_code = CCErrorCode::ERROR_GROUPS_GROUP_COMMAND_UNKNOWN;
        break;
    }
    return false;
}

}  // namespace Kiran
