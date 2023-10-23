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

#include "plugins/power/idle/power-idle-xalarm.h"
#include "plugins/power/wrapper/power-wrapper-manager.h"

namespace Kiran
{
enum PowerIdleMode
{
    // 正常，不做任何操作
    POWER_IDLE_MODE_NORMAL,
    // 需要将显示设备调暗
    POWER_IDLE_MODE_DIM,
    // 需要将显示设备设为节能模式，节能模式下显示设备变黑
    POWER_IDLE_MODE_BLANK,
    // 需要将计算机设为节能模式
    POWER_IDLE_MODE_SLEEP
};

// 空闲模式计时器
class PowerIdleTimer
{
public:
    PowerIdleTimer();
    virtual ~PowerIdleTimer();

    void init();

    /* 当mode为POWER_IDLE_MODE_DIM时，设置timeout秒内没有(鼠标、键盘等）操作则发送显示设备需要变暗信号(POWER_IDLE_MODE_DIM)；
       当mode为POWER_IDLE_MODE_BLANK时，如果显示设备已变暗且持续timeout(秒)还是处于空闲状态，则发送显示设备需要进入节能模式信号(POWER_IDLE_MODE_BLANK)；
       当mode为POWER_IDLE_MODE_SLEEP时，在显示设备已变暗且会话处于空闲状态后，持续timeout(秒)还是处于空闲状态，则发送计算机进入节能模式信号(POWER_IDLE_MODE_SLEEP)。
       一旦用户通过鼠标、键盘等操作重新激活计算机，则发送POWER_IDLE_MODE_NORMAL信号，且所有定时器重新开始计算*/
    bool set_idle_timeout(PowerIdleMode mode, uint32_t timeout);

    PowerIdleMode get_idle_mode() { return this->mode_; };

    // 发送模式变化的信号
    sigc::signal<void, PowerIdleMode>& signal_mode_changed() { return this->mode_changed_; };

private:
    bool set_dim_timeout(uint32_t timeout);
    bool set_blank_timeout(uint32_t timeout);
    bool set_sleep_timeout(uint32_t timeout);

    // 更新空闲模式
    void update_mode();
    void switch_mode(PowerIdleMode mode);

    // 移除定时器
    void remove_blank_timeout();
    void remove_sleep_timeout();

    // 定时器回调函数
    bool on_blank_timeout_cb();
    bool on_sleep_timeout_cb();

    void on_session_idle_status_changed(bool is_idle);
    void on_inhibitor_changed();

    void on_alarm_triggered(std::shared_ptr<XAlarmInfo> xalarm);
    void on_alarm_reset();

    std::string idle_mode_enum2str(PowerIdleMode mode);

private:
    PowerIdleMode mode_;
    sigc::signal<void, PowerIdleMode> mode_changed_;

    std::shared_ptr<PowerSession> session_;
    PowerIdleXAlarm idle_xalarm_;

    bool is_xidle_;

    uint32_t blank_timeout_;
    sigc::connection blank_timeout_id_;

    uint32_t sleep_timeout_;
    sigc::connection sleep_timeout_id_;
};
}  // namespace Kiran