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

#include "plugins/power/idle/power-idle-timer.h"

namespace Kiran
{
PowerIdleTimer::PowerIdleTimer() : mode_(PowerIdleMode::POWER_IDLE_MODE_NORMAL),
                                   is_xidle_(false),
                                   blank_timeout_(0),
                                   sleep_timeout_(0)
{
    this->session_ = PowerWrapperManager::get_instance()->get_default_session();
}

PowerIdleTimer::~PowerIdleTimer()
{
    this->remove_blank_timeout();
    this->remove_sleep_timeout();
}

void PowerIdleTimer::init()
{
    this->idle_xalarm_.init();

    this->session_->signal_idle_status_changed().connect(sigc::mem_fun(this, &PowerIdleTimer::on_session_idle_status_changed));
    this->session_->signal_inhibitor_changed().connect(sigc::mem_fun(this, &PowerIdleTimer::on_inhibitor_changed));

    this->idle_xalarm_.signal_alarm_triggered().connect(sigc::mem_fun(this, &PowerIdleTimer::on_alarm_triggered));
    this->idle_xalarm_.signal_alarm_reset().connect(sigc::mem_fun(this, &PowerIdleTimer::on_alarm_reset));
}

bool PowerIdleTimer::set_idle_timeout(PowerIdleMode mode, uint32_t timeout)
{
    KLOG_DEBUG_POWER("Set idle timeout to %d for mode %s.", timeout, this->idle_mode_enum2str(mode).c_str());
    switch (mode)
    {
    case PowerIdleMode::POWER_IDLE_MODE_DIM:
        return this->set_dim_timeout(timeout);
    case PowerIdleMode::POWER_IDLE_MODE_BLANK:
        return this->set_blank_timeout(timeout);
    case PowerIdleMode::POWER_IDLE_MODE_SLEEP:
        return this->set_sleep_timeout(timeout);
    default:
        break;
    }
    return false;
}

std::string PowerIdleTimer::idle_mode_enum2str(PowerIdleMode mode)
{
    std::string mode_info;
    switch (mode)
    {
    case PowerIdleMode::POWER_IDLE_MODE_NORMAL:
        mode_info = "normal";
        break;
    case PowerIdleMode::POWER_IDLE_MODE_DIM:
        mode_info = "dim";
        break;
    case PowerIdleMode::POWER_IDLE_MODE_BLANK:
        mode_info = "blank";
        break;
    case PowerIdleMode::POWER_IDLE_MODE_SLEEP:
        mode_info = "sleep";
        break;
    default:
        mode_info = "";
        break;
    }
    return mode_info;
}

bool PowerIdleTimer::set_dim_timeout(uint32_t timeout)
{
    auto xidle_time = this->idle_xalarm_.get_xidle_time() / 1000;

    /*如果设置的超时时间比计算机已经空闲的时间小，则报警器信号无法被触发，
    因此这里设置请求的超时时间比已经空闲的时间至少大10秒钟*/
    if (timeout > 0 && timeout <= xidle_time + 10)
    {
        timeout = xidle_time + 10;
    }

    if (timeout > 0)
    {
        this->idle_xalarm_.set(XAlarmType::XALARM_TYPE_DIM, timeout * 1000);
    }
    else
    {
        this->idle_xalarm_.unset(XAlarmType::XALARM_TYPE_DIM);
    }

    return true;
}

bool PowerIdleTimer::set_blank_timeout(uint32_t timeout)
{
    this->blank_timeout_ = timeout;
    this->update_mode();
    return true;
}

bool PowerIdleTimer::set_sleep_timeout(uint32_t timeout)
{
    this->sleep_timeout_ = timeout;
    this->update_mode();
    return true;
}

void PowerIdleTimer::update_mode()
{
    auto is_idle = this->session_->get_idle();
    auto idle_inhibit = this->session_->get_idle_inhibited();

    KLOG_DEBUG_POWER("is_idle: %d idle_inhibit: %d.", is_idle, idle_inhibit);
    // 如果为未空闲状态，或者禁止空闲时操作，则不进行节能处理
    if (!is_idle || idle_inhibit)
    {
        this->switch_mode(PowerIdleMode::POWER_IDLE_MODE_NORMAL);
        this->remove_blank_timeout();
        this->remove_sleep_timeout();
        return;
    }

    // 计算机空闲超时后首先进入显示器变暗状态
    if (this->mode_ == PowerIdleMode::POWER_IDLE_MODE_NORMAL)
    {
        this->switch_mode(PowerIdleMode::POWER_IDLE_MODE_DIM);
    }

    // 设置空闲一定时间后显示器/计算机进入节能模式的定时器
    if (!this->blank_timeout_id_ && this->blank_timeout_ > 0)
    {
        auto timeout = Glib::MainContext::get_default()->signal_timeout();
        this->blank_timeout_id_ = timeout.connect_seconds(sigc::mem_fun(this, &PowerIdleTimer::on_blank_timeout_cb), this->blank_timeout_);
    }

    auto is_suspend_inhibit = this->session_->get_suspend_inhibited();
    if (is_suspend_inhibit)
    {
        this->remove_sleep_timeout();
    }
    else if (!this->sleep_timeout_id_ &&
             this->sleep_timeout_ > 0)
    {
        auto timeout = Glib::MainContext::get_default()->signal_timeout();
        this->sleep_timeout_id_ = timeout.connect_seconds(sigc::mem_fun(this, &PowerIdleTimer::on_sleep_timeout_cb), this->sleep_timeout_);
    }
}

void PowerIdleTimer::switch_mode(PowerIdleMode mode)
{
    if (this->mode_ != mode)
    {
        this->mode_ = mode;
        this->mode_changed_.emit(mode);
    }
}

void PowerIdleTimer::remove_blank_timeout()
{
    if (this->blank_timeout_id_)
    {
        this->blank_timeout_id_.disconnect();
    }
}

void PowerIdleTimer::remove_sleep_timeout()
{
    if (this->sleep_timeout_id_)
    {
        this->sleep_timeout_id_.disconnect();
    }
}

bool PowerIdleTimer::on_blank_timeout_cb()
{
    // 如果已经进入计算机节能模式，则不再发送显示设备节能信号
    if (this->mode_ >= PowerIdleMode::POWER_IDLE_MODE_BLANK)
    {
        KLOG_DEBUG_POWER("Ignore blank timeout, mode is %d.", this->mode_);
        return false;
    }

    this->switch_mode(PowerIdleMode::POWER_IDLE_MODE_BLANK);
    return false;
}

bool PowerIdleTimer::on_sleep_timeout_cb()
{
    this->switch_mode(PowerIdleMode::POWER_IDLE_MODE_SLEEP);
    return false;
}

void PowerIdleTimer::on_session_idle_status_changed(bool is_idle)
{
    KLOG_DEBUG_POWER("On session idle status changed");
    this->update_mode();
}

void PowerIdleTimer::on_inhibitor_changed()
{
    this->update_mode();
}

void PowerIdleTimer::on_alarm_triggered(std::shared_ptr<XAlarmInfo> xalarm)
{
    this->is_xidle_ = true;
}

void PowerIdleTimer::on_alarm_reset()
{
    this->is_xidle_ = false;
}

}  // namespace Kiran