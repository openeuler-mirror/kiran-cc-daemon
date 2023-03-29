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

#include "plugins/power/save/power-save-computer.h"

namespace Kiran
{
PowerSaveComputer::PowerSaveComputer()
{
    this->power_settings_ = Gio::Settings::create(POWER_SCHEMA_ID);
    this->login1_ = PowerWrapperManager::get_instance()->get_default_login1();
    this->screensaver_ = PowerWrapperManager::get_instance()->get_default_screensaver();
}

void PowerSaveComputer::init()
{
}

void PowerSaveComputer::suspend()
{
    uint32_t throttle = 0;

    auto lockscreen = this->power_settings_->get_boolean(POWER_SCHEMA_SCREEN_LOCKED_WHEN_SUSPEND);
    // 挂起之前锁定屏幕
    if (lockscreen)
    {
        throttle = this->screensaver_->lock_and_throttle("suspend");
    }

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
    uint32_t throttle = 0;

    auto lockscreen = this->power_settings_->get_boolean(POWER_SCHEMA_SCREEN_LOCKED_WHEN_HIBERNATE);
    // 休眠之前锁定屏幕
    if (lockscreen)
    {
        throttle = this->screensaver_->lock_and_throttle("hibernate");
    }

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
    this->login1_->shutdown();
}

}  // namespace Kiran
