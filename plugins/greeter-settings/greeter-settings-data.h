/**
 * @file greeter-settings-data.h
 * @brief 登录界面配置项数据
 * @author songchuanfei <songchuanfei@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved.
 */

#ifndef GREETER_SETTINGS_DATA_H
#define GREETER_SETTINGS_DATA_H

#include "greeter-settings_i.h"
#include <glibmm.h>

/**
 * @brief 登录界面配置项数据
 */
class GreeterSettingsData {
public:
    GreeterSettingsData();
    GreeterSettingsData(GreeterSettingsData &data_);
    void clear();

public:
    GreeterSettingsScalingMode scale_mode;                  /**< 缩放模式 */
    uint32_t autologin_delay;                       /**< 自动登录延时,单位为秒 */

    uint32_t scale_factor;                          /**< 界面缩放比例, 1表示100%, 2表示200% */
    bool enable_manual_login;                       /**< 是否允许手动输入用户名登录 */
    bool hide_user_list;                            /**< 是否隐藏用户列表 */
    Glib::ustring autologin_user;                   /**< 自动登录用户名 */
    Glib::ustring background_file;                  /**< 登录界面背景图片 */
};


#endif //GREETER_SETTINGS_DATA_H
