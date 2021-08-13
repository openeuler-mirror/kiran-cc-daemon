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

#pragma once

#include "plugins/power/wrapper/power-wrapper-manager.h"
#include "power-i.h"

namespace Kiran
{
enum ComputerSaveState
{
    // 进入节能状态
    COMPUTER_SAVE_STATE_SLEEP,
    // 从节能状态中恢复
    COMPUTER_SAVE_STATE_RESUME,
};

// 计算机节能
class PowerSaveComputer
{
public:
    PowerSaveComputer();
    virtual ~PowerSaveComputer(){};

    void init();

    // 挂机
    void suspend();
    // 休眠
    void hibernate();
    // 关机
    void shutdown();

private:
    sigc::signal<void, ComputerSaveState, PowerAction>& signal_save_changed() { return this->save_changed_; }

private:
    static PowerSaveComputer* instance_;

    std::shared_ptr<PowerLogin1> login1_;
    std::shared_ptr<PowerScreenSaver> screensaver_;

    sigc::signal<void, ComputerSaveState, PowerAction> save_changed_;
};
}  // namespace  Kiran
