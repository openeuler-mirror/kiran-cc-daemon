/**
 * @file greeter-settings-data.h
 * @brief 登录界面配置项数据
 * @author songchuanfei <songchuanfei@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved.
 */

#ifndef GREETER_SETTINGS_DATA_H
#define GREETER_SETTINGS_DATA_H

#include <glibmm.h>

/**
 * 登录界面缩放模式
 */
typedef enum
{
    SCALING_AUTO = 0, /** 自动缩放, 根据屏幕分辨率自动调整缩放率 */
    SCALING_MANUAL,   /** 手动设定缩放率 */
    SCALING_DISABLE,  /** 禁用缩放 */
    SCALING_LAST,
} GreeterScalingMode;

/**
 * @brief 登录界面配置项数据
 */
class GreeterSettingsData {
public:
    GreeterSettingsData();
    GreeterSettingsData(GreeterSettingsData &data_);

public:
    GreeterScalingMode scale_mode;                  /**< 缩放模式 */
    uint32_t autologin_delay;                       /**< 自动登录延时,单位为秒 */

    uint32_t scale_factor;                          /**< 界面缩放比例, 1表示100%, 2表示200% */
    bool enable_manual_login;                       /**< 是否允许手动输入用户名登录 */
    bool hide_user_list;                            /**< 是否隐藏用户列表 */
    Glib::ustring autologin_user;                   /**< 自动登录用户名 */
    Glib::ustring background_file;                  /**< 登录界面背景图片 */
};


#endif //GREETER_SETTINGS_DATA_H