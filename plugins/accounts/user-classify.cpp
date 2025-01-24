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

#include "user-classify.h"
#include <unistd.h>
#include <QFileInfo>

namespace Kiran
{
#define MINIMUM_UID 1000

const QSet<QString> UserClassify::m_defaultExcludes = {
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

bool UserClassify::isHuman(uint32_t uid, const QString &userName, const QString &shell)
{
    if (UserClassify::m_defaultExcludes.find(userName) != UserClassify::m_defaultExcludes.end())
    {
        return false;
    }

    if (!shell.isEmpty() && isInvalidShell(shell))
        return false;

    return uid >= MINIMUM_UID;
}

bool UserClassify::isInvalidShell(const QString &shell)
{
    int ret = false;

#ifdef HAVE_GETUSERSHELL
    /* getusershell returns a whitelist of valid shells. assume the shell is invalid unless there is a match */
    ret = true;
    char *validShell;

    setusershell();
    while ((validShell = getusershell()) != NULL)
    {
        if (g_strcmp0(shell.c_str(), validShell) != 0)
            continue;
        ret = false;
        break;
    }
    endusershell();
#endif

    /* always check for false and nologin since they are sometimes included by getusershell */
    auto basename = QFileInfo(shell).baseName();
    if (shell.isEmpty() || basename == "nologin" || basename == "false")
    {
        return true;
    }

    return ret;
}

}  // namespace Kiran