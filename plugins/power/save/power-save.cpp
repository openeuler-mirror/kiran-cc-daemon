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
    KLOG_PROFILE("action: %d.", action);

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
    // TODO:
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
}  // namespace Kiran