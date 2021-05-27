/**
 * @file          /kiran-cc-daemon/include/greeter_i.h
 * @brief description
 * @author yangxiaoqing <yangxiaoqing@kylinos.com.cn>
 * @copyright (c) 2021 KylinSec. All rights reserved.
*/
#pragma once

#ifndef GREETER_NEW_INTERFACE
#warning This file will be deprecated. please use greeter-i.h file
#endif

#define GREETER_DBUS_NAME "com.kylinsec.Kiran.SystemDaemon.Greeter"
#define GREETER_OBJECT_PATH "/com/kylinsec/Kiran/SystemDaemon/Greeter"

#ifdef __cplusplus
extern "C"
{
#endif

    // 登录界面缩放模式
    enum GreeterScalingMode
    {
        // 手动设定缩放率
        GREETER_SCALING_MODE_AUTO = 0,
        // 手动设定缩放率
        GREETER_SCALING_MODE_MANUAL,
        // 禁用缩放
        GREETER_SCALING_MODE_DISABLE,
        GREETER_SCALING_MODE_LAST,
    };

#ifdef __cplusplus
}
#endif
