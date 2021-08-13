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

#include "plugins/timedate/timedate-util.h"

#include <fcntl.h>
#include <glib/gstdio.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "lib/base/base.h"
#include "plugins/timedate/timedate-def.h"

namespace Kiran
{
bool TimedateUtil::is_local_rtc()
{
    std::string contents;
    try
    {
        contents = Glib::file_get_contents(ADJTIME_PATH);
    }
    catch (const Glib::FileError &e)
    {
        return false;
    }

    if (contents.find("LOCAL") != std::string::npos)
    {
        return true;
    }
    return false;
}

std::string TimedateUtil::get_timezone()
{
    g_autofree gchar *link = NULL;

    link = g_file_read_link(LOCALTIME_PATH, NULL);
    if (!link)
    {
        return std::string();
    }

    auto zone = g_strrstr(link, ZONEINFO_PATH);
    if (!zone)
    {
        return std::string();
    }

    zone += strlen(ZONEINFO_PATH);

    return std::string(zone);
}

int64_t TimedateUtil::get_gmt_offset(const std::string &zone)
{
    char *tz = getenv("TZ");
    setenv("TZ", zone.c_str(), 1);

    time_t cur_time = time(NULL);
    auto tm = localtime(&cur_time);

    if (tz)
    {
        setenv("TZ", tz, 1);
    }
    else
    {
        unsetenv("TZ");
    }

    if (!tm)
    {
        return -1;
    }

#if defined __USE_MISC
    return tm->tm_gmtoff;
#else
    return tm->__tm_gmtoff;
#endif
}
}  // namespace Kiran
