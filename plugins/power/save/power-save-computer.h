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
