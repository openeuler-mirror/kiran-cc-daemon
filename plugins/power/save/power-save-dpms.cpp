/**
 * @Copyright (C) 2020 ~ 2021 KylinSec Co., Ltd. 
 *
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; If not, see <http: //www.gnu.org/licenses/>. 
 */

#include "plugins/power/save/power-save-dpms.h"

namespace Kiran
{
#define POWER_DPMS_TIMING_CHECK 10

PowerSaveDpms::PowerSaveDpms() : capable_(false),
                                 cached_level_(PowerDpmsLevel::POWER_DPMS_LEVEL_UNKNOWN)
{
    this->display_ = gdk_display_get_default();
    this->xdisplay_ = GDK_DISPLAY_XDISPLAY(this->display_);
}

PowerSaveDpms::~PowerSaveDpms()
{
    if (this->timing_handler_)
    {
        this->timing_handler_.disconnect();
    }
}

void PowerSaveDpms::init()
{
    RETURN_IF_TRUE(this->xdisplay_ == NULL);

    this->capable_ = DPMSCapable(this->xdisplay_);
    KLOG_DEBUG("capable: %d.", this->capable_);

    // 由于dpms协议未提供power levels变化的信号，因此这里采用定时检测的方式判断power levels是否变化
    auto timeout = Glib::MainContext::get_default()->signal_timeout();
    this->timing_handler_ = timeout.connect_seconds(sigc::mem_fun(this, &PowerSaveDpms::on_timing_check_level_cb), POWER_DPMS_TIMING_CHECK);

    this->clear_dpms_timeout();
}

bool PowerSaveDpms::set_level(PowerDpmsLevel level)
{
    CARD16 state;
    CARD16 current_state;
    BOOL current_enabled;

    RETURN_VAL_IF_FALSE(this->capable_, false);

    if (!DPMSInfo(this->xdisplay_, &current_state, &current_enabled))
    {
        KLOG_WARNING("Couldn't get DPMS info");
        return false;
    }

    // dpms功能未开启
    if (!current_enabled)
    {
        KLOG_WARNING("DPMS not enabled");
        return false;
    }

    auto current_level = this->get_level();

    if (current_level != level)
    {
        state = this->level_enum2card(level);

        if (!DPMSForceLevel(this->xdisplay_, state))
        {
            KLOG_WARNING("Couldn't change DPMS mode");
            return false;
        }
        XSync(this->xdisplay_, FALSE);
    }

    /* 可能出现的情况：本程序设置为POWER_DPMS_LEVEL_STANDBY->其他程序设置为POWER_DPMS_LEVEL_ON->本程序又设置回POWER_DPMS_LEVEL_STANDBY，
    但是cached_level没有及时更新还是为POWER_DPMS_LEVEL_STANDBY，此时
    */
    if (level != this->cached_level_)
    {
        this->cached_level_ = level;
        this->level_changed_.emit(this->cached_level_);
    }

    return true;
}

PowerDpmsLevel PowerSaveDpms::get_level()
{
    BOOL enabled = FALSE;
    CARD16 state;

    RETURN_VAL_IF_FALSE(this->capable_, PowerDpmsLevel::POWER_DPMS_LEVEL_UNKNOWN);

    DPMSInfo(this->xdisplay_, &state, &enabled);
    RETURN_VAL_IF_FALSE(enabled, PowerDpmsLevel::POWER_DPMS_LEVEL_UNKNOWN);

    return this->level_card2enum(state);
}

void PowerSaveDpms::clear_dpms_timeout()
{
    RETURN_IF_FALSE(this->capable_);
    DPMSSetTimeouts(this->xdisplay_, 0, 0, 0);
}

CARD16 PowerSaveDpms::level_enum2card(PowerDpmsLevel enum_level)
{
    switch (enum_level)
    {
    case PowerDpmsLevel::POWER_DPMS_LEVEL_ON:
        return DPMSModeOn;
    case PowerDpmsLevel::POWER_DPMS_LEVEL_STANDBY:
        return DPMSModeStandby;
    case PowerDpmsLevel::POWER_DPMS_LEVEL_SUSPEND:
        return DPMSModeSuspend;
    case PowerDpmsLevel::POWER_DPMS_LEVEL_OFF:
        return DPMSModeOff;
    default:
        break;
    }
    return DPMSModeOn;
}

PowerDpmsLevel PowerSaveDpms::level_card2enum(CARD16 card_level)
{
    switch (card_level)
    {
    case DPMSModeOn:
        return PowerDpmsLevel::POWER_DPMS_LEVEL_ON;
    case DPMSModeStandby:
        return PowerDpmsLevel::POWER_DPMS_LEVEL_STANDBY;
    case DPMSModeSuspend:
        return PowerDpmsLevel::POWER_DPMS_LEVEL_SUSPEND;
    case DPMSModeOff:
        return PowerDpmsLevel::POWER_DPMS_LEVEL_OFF;
    default:
        break;
    }
    return PowerDpmsLevel::POWER_DPMS_LEVEL_UNKNOWN;
}

bool PowerSaveDpms::on_timing_check_level_cb()
{
    auto current_level = this->get_level();

    if (current_level != this->cached_level_)
    {
        this->cached_level_ = current_level;
        this->level_changed_.emit(this->cached_level_);
    }
    return true;
}

}  // namespace Kiran