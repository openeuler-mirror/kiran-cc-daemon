/*
 * @Author       : tangjie02
 * @Date         : 2020-08-31 14:42:39
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-08-31 14:51:57
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/plugins/timedate/timedate-util.cpp
 */

#include "plugins/timedate/timedate-util.h"

namespace Kiran
{
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
