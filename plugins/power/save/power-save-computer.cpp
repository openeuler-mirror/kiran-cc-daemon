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
    this->session_ = PowerWrapperManager::get_instance()->get_default_session();
}

void PowerSaveComputer::init()
{
}

void PowerSaveComputer::suspend()
{
    this->save_changed_.emit(ComputerSaveState::COMPUTER_SAVE_STATE_SLEEP, PowerAction::POWER_ACTION_COMPUTER_SUSPEND);
    this->session_->suspend();
    this->save_changed_.emit(ComputerSaveState::COMPUTER_SAVE_STATE_RESUME, PowerAction::POWER_ACTION_COMPUTER_SUSPEND);
}

void PowerSaveComputer::hibernate()
{
    this->save_changed_.emit(ComputerSaveState::COMPUTER_SAVE_STATE_SLEEP, PowerAction::POWER_ACTION_COMPUTER_HIBERNATE);
    this->session_->hibernate();
    this->save_changed_.emit(ComputerSaveState::COMPUTER_SAVE_STATE_RESUME, PowerAction::POWER_ACTION_COMPUTER_HIBERNATE);
}

void PowerSaveComputer::shutdown()
{
    this->session_->shutdown();
}

}  // namespace Kiran
