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
        TOUCHPAD_HOUSR_FORMAT_12_HOURS = 0,
        // 24小时制
        TOUCHPAD_HOUSR_FORMAT_24_HOURS,
        TOUCHPAD_HOUSR_FORMAT_LAST
    };

#ifdef __cplusplus
}
#endif