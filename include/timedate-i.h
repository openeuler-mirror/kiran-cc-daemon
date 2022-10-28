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