/*
 * @Author       : tangjie02
 * @Date         : 2020-07-23 14:29:54
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-07-24 13:48:16
 * @Description  : 
 * @FilePath     : /kiran-system-daemon/plugins/accounts/user-classify.cpp
 */

#include "plugins/accounts/user-classify.h"

#include <glibmm.h>
#include <unistd.h>

namespace Kiran
{
#define MINIMUM_UID 1000

const std::set<std::string> UserClassify::default_excludes_ = {
    "bin",
    "root",
    "daemon",
    "adm",
    "lp",
    "sync",
    "shutdown",
    "halt",
    "mail",
    "news",
    "uucp",
    "nobody",
    "postgres",
    "pvm",
    "rpm",
    "nfsnobody",
    "pcap",
    "mysql",
    "ftp",
    "games",
    "man",
    "at",
    "gdm",
    "gnome-initial-setup"};

bool UserClassify::is_human(uint32_t uid,
                            const std::string &username,
                            const std::string &shell)
{
    if (UserClassify::default_excludes_.find(username) != UserClassify::default_excludes_.end())
    {
        return false;
    }

    if (!shell.empty() && is_invalid_shell(shell))
        return false;

    return uid >= MINIMUM_UID;
}

bool UserClassify::is_invalid_shell(const std::string &shell)
{
    int ret = false;

#ifdef HAVE_GETUSERSHELL
    /* getusershell returns a whitelist of valid shells. assume the shell is invalid unless there is a match */
    ret = true;
    char *valid_shell;

    setusershell();
    while ((valid_shell = getusershell()) != NULL)
    {
        if (g_strcmp0(shell.c_str(), valid_shell) != 0)
            continue;
        ret = false;
        break;
    }
    endusershell();
#endif

    /* always check for false and nologin since they are sometimes included by getusershell */
    auto basename = Glib::path_get_basename(shell);
    if (shell.empty() || basename == "nologin" || basename == "false")
    {
        return true;
    }

    return ret;
}

}  // namespace Kiran