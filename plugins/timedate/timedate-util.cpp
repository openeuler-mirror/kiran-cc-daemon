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

#include "plugins/timedate/timedate-util.h"
#include <glib.h>
#include <QFile>
#include "lib/base/base.h"
#include "timedate-def.h"

namespace Kiran
{
bool TimedateUtil::isLocalRtc()
{
    QFile file(ADJTIME_PATH);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        KLOG_WARNING(timedate) << "Cannot access file" << ADJTIME_PATH;
        return false;
    }

    auto contents = file.readAll();
    return contents.contains("LOCAL");
}

QString TimedateUtil::getTimezone()
{
    g_autofree gchar *link = NULL;

    link = g_file_read_link(LOCALTIME_PATH, NULL);
    if (!link)
    {
        return QString();
    }

    auto zone = g_strrstr(link, ZONEINFO_PATH);
    if (!zone)
    {
        return QString();
    }

    zone += strlen(ZONEINFO_PATH);

    return QString(zone);
}

int64_t TimedateUtil::getGMTOffset(const QString &zone)
{
    char *tz = getenv("TZ");
    setenv("TZ", zone.toLatin1().data(), 1);

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
