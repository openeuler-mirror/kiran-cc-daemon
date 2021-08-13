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

#ifndef SYSTEMINFO_NEW_INTERFACE
#warning This file will be deprecated. please use systeminfo-i.h file
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#define SYSTEMINFO_DBUS_NAME "com.kylinsec.Kiran.SystemDaemon.SystemInfo"
#define SYSTEMINFO_OBJECT_PATH "/com/kylinsec/Kiran/SystemDaemon/SystemInfo"
#define SYSTEMINFO_DBUS_INTERFACE_NAME "com.kylinsec.Kiran.SystemDaemon.SystemInfo"

    enum SystemInfoType
    {
        // 软件信息，包括主机名、内核版本、系统架构等信息。
        SYSTEMINFO_TYPE_SOFTWARE = 0,
        // 硬件信息，包括CPU、显卡、内存、网卡、硬盘等硬件信息。
        SYSTEMINFO_TYPE_HARDWARE,
        SYSTEMINFO_TYPE_LAST
    };

#ifdef __cplusplus
}
#endif