
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
 * Author:     meizhigang <meizhigang@kylinsec.com.cn>
 */
#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef KCC_SYSTEM_TYPE
#define CC_DAEMON_DBUS_NAME "com.kylinsec.Kiran.SystemDaemon"
#define CC_DAEMON_OBJECT_PATH "/com/kylinsec/Kiran/SystemDaemon"
#elif KCC_SESSION_TYPE
#define CC_DAEMON_DBUS_NAME "com.kylinsec.Kiran.SessionDaemon"
#define CC_DAEMON_OBJECT_PATH "/com/kylinsec/Kiran/SessionDaemon"
#else
#define CC_DAEMON_DBUS_NAME "com.kylinsec.Kiran.CCDaemon"
#define CC_DAEMON_OBJECT_PATH "/com/kylinsec/Kiran/CCDaemon"
#error need to define KCC_SYSTEM_TYPE or KCC_SESSION_TYPE
#endif

#ifdef __cplusplus
}
#endif