/**
 * @file          /kiran-cc-daemon/include/systeminfo_i.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */
#pragma once

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