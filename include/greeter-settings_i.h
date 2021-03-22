/**
 * @file greeter-settings_i.h
 * @brief description
 * @author yangxiaoqing <yangxiaoqing@kylinos.com.cn>
 * @copyright (c) 2021 KylinSec. All rights reserved.
*/
#pragma once

#define GREETER_SETTINGS_DBUS_NAME     "com.kylinsec.Kiran.SystemDaemon.GreeterSettings"
#define GREETER_SETTINGS_OBJECT_PATH   "/com/kylinsec/Kiran/SystemDaemon/GreeterSettings"
#define AUTH_SET_LOGIN_OPTION          "com.kylinsec.kiran.system-daemon.greeter-settings.set-login-option"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * 登录界面缩放模式
 */
typedef enum
{
    GREETER_SETTINGS_SCALING_MODE_AUTO = 0, /** 自动缩放, 根据屏幕分辨率自动调整缩放率 */
    GREETER_SETTINGS_SCALING_MODE_MANUAL,   /** 手动设定缩放率 */
    GREETER_SETTINGS_SCALING_MODE_DISABLE,  /** 禁用缩放 */
    GREETER_SETTINGS_SCALING_MODE_LAST,
} GreeterSettingsScalingMode;

#ifdef __cplusplus
}
#endif
