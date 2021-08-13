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

#include "plugins/power/save/power-save-computer.h"

namespace Kiran
{
PowerSaveComputer::PowerSaveComputer()
{
    this->login1_ = PowerWrapperManager::get_instance()->get_default_login1();
    this->screensaver_ = PowerWrapperManager::get_instance()->get_default_screensaver();
}

void PowerSaveComputer::init()
{
}

void PowerSaveComputer::suspend()
{
    KLOG_PROFILE("");

    // 挂起之前锁定屏幕
    auto throttle = this->screensaver_->lock_and_throttle("suspend");

    this->save_changed_.emit(ComputerSaveState::COMPUTER_SAVE_STATE_SLEEP, PowerAction::POWER_ACTION_COMPUTER_SUSPEND);
    this->login1_->suspend();
    this->save_changed_.emit(ComputerSaveState::COMPUTER_SAVE_STATE_RESUME, PowerAction::POWER_ACTION_COMPUTER_SUSPEND);

    this->screensaver_->poke();
    if (throttle)
    {
        this->screensaver_->remove_throttle(throttle);
    }
}

void PowerSaveComputer::hibernate()
{
    KLOG_PROFILE("");

    // 休眠之前锁定屏幕
    auto throttle = this->screensaver_->lock_and_throttle("hibernate");

    this->save_changed_.emit(ComputerSaveState::COMPUTER_SAVE_STATE_SLEEP, PowerAction::POWER_ACTION_COMPUTER_HIBERNATE);
    this->login1_->hibernate();
    this->save_changed_.emit(ComputerSaveState::COMPUTER_SAVE_STATE_RESUME, PowerAction::POWER_ACTION_COMPUTER_HIBERNATE);

    this->screensaver_->poke();
    if (throttle)
    {
        this->screensaver_->remove_throttle(throttle);
    }
}

void PowerSaveComputer::shutdown()
{
    KLOG_PROFILE("");

    this->login1_->shutdown();
}

}  // namespace Kiran
