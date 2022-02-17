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

#include "plugins/power/save/power-save.h"

namespace Kiran
{
PowerSave::PowerSave(PowerWrapperManager* wrapper_manager) : wrapper_manager_(wrapper_manager)
{
}

PowerSave::~PowerSave()
{
}

PowerSave* PowerSave::instance_ = nullptr;
void PowerSave::global_init(PowerWrapperManager* wrapper_manager)
{
    instance_ = new PowerSave(wrapper_manager);
    instance_->init();
}

bool PowerSave::do_save(PowerAction action, std::string& error)
{
    KLOG_PROFILE("Do action: %s.", this->action2str(action).c_str());

    switch (action)
    {
    case PowerAction::POWER_ACTION_DISPLAY_ON:
        this->save_dpms_.set_level(PowerDpmsLevel::POWER_DPMS_LEVEL_ON);
        break;
    case PowerAction::POWER_ACTION_DISPLAY_STANDBY:
        this->save_dpms_.set_level(PowerDpmsLevel::POWER_DPMS_LEVEL_STANDBY);
        break;
    case PowerAction::POWER_ACTION_DISPLAY_SUSPEND:
        this->save_dpms_.set_level(PowerDpmsLevel::POWER_DPMS_LEVEL_SUSPEND);
        break;
    case PowerAction::POWER_ACTION_DISPLAY_OFF:
        this->save_dpms_.set_level(PowerDpmsLevel::POWER_DPMS_LEVEL_OFF);
        break;
    case PowerAction::POWER_ACTION_COMPUTER_SUSPEND:
        this->save_computer_.suspend();
        break;
    case PowerAction::POWER_ACTION_COMPUTER_SHUTDOWN:
        this->save_computer_.shutdown();
        break;
    case PowerAction::POWER_ACTION_COMPUTER_HIBERNATE:
        this->save_computer_.hibernate();
        break;
    case PowerAction::POWER_ACTION_NOTHING:
        break;
    default:
        error = "Unsupported action";
        return false;
    }
    return true;
}

void PowerSave::init()
{
    this->save_computer_.init();
    this->save_dpms_.init();
}

std::string PowerSave::action2str(PowerAction action)
{
    switch (action)
    {
    case PowerAction::POWER_ACTION_DISPLAY_ON:
        return "display on";
    case PowerAction::POWER_ACTION_DISPLAY_STANDBY:
        return "display standby";
    case PowerAction::POWER_ACTION_DISPLAY_SUSPEND:
        return "display suspend";
    case PowerAction::POWER_ACTION_DISPLAY_OFF:
        return "display off";
    case PowerAction::POWER_ACTION_COMPUTER_SUSPEND:
        return "computer suspend";
    case PowerAction::POWER_ACTION_COMPUTER_SHUTDOWN:
        return "computer shutdown";
    case PowerAction::POWER_ACTION_COMPUTER_HIBERNATE:
        return "computer hibernate";
    case PowerAction::POWER_ACTION_NOTHING:
        return "nothing";
    default:
        break;
    };
    return "unknown action";
}
}  // namespace Kiran