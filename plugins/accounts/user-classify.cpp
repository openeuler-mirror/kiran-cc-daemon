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