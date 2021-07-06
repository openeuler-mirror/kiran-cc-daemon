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


#pragma once

#ifndef TIMEDATE_NEW_INTERFACE
#warning This file will be deprecated. please use timedate-i.h file
#endif

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