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