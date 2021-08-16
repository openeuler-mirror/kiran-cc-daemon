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
