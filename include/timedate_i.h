/**
 * @file          /kiran-cc-daemon/include/timedate_i.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#define TIMEDATE_DBUS_NAME "com.kylinsec.Kiran.SystemDaemon.TimeDate"
#define TIMEDATE_OBJECT_PATH "/com/kylinsec/Kiran/SystemDaemon/TimeDate"
#define TIMEDATE_DBUS_INTERFACE_NAME "com.kylinsec.Kiran.SystemDaemon.TimeDate"

    enum TimedateDateFormatType
    {
        // 时间日期的长格式
        TIMEDATE_FORMAT_TYPE_LONG = 0,
        // 时间日期的短格式
        TIMEDATE_FORMAT_TYPE_SHORT,
        TIMEDATE_FORMAT_TYPE_LAST,
    };

    enum TimedateHourFormat
    {
        // 12小时制
        TIMEDATE_HOUSR_FORMAT_12_HOURS = 0,
        // 24小时制
        TIMEDATE_HOUSR_FORMAT_24_HOURS,
        TIMEDATE_HOUSR_FORMAT_LAST
    };

#ifdef __cplusplus
}
#endif