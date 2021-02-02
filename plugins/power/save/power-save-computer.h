/**
 * @file          /kiran-cc-daemon/plugins/power/save/power-save-computer.h
 * @brief         计算机节能
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */
#pragma once

#include "plugins/power/wrapper/power-wrapper-manager.h"
#include "power_i.h"

namespace Kiran
{
enum ComputerSaveState
{
    // 进入节能状态
    COMPUTER_SAVE_STATE_SLEEP,
    // 从节能状态中恢复
    COMPUTER_SAVE_STATE_RESUME,
};

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
