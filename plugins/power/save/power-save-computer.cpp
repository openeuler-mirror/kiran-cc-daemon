/**
 * @file          /kiran-cc-daemon/plugins/power/save/power-save-computer.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
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
    SETTINGS_PROFILE("");

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
    SETTINGS_PROFILE("");

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
    SETTINGS_PROFILE("");

    this->login1_->shutdown();
}

}  // namespace Kiran
