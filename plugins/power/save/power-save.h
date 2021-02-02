/**
 * @file          /kiran-cc-daemon/plugins/power/save/power-save.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#pragma once

#include "plugins/power/save/power-save-computer.h"
#include "plugins/power/save/power-save-dpms.h"
#include "plugins/power/wrapper/power-wrapper-manager.h"
#include "power_i.h"

namespace Kiran
{
class PowerSave
{
public:
    PowerSave(PowerWrapperManager* wrapper_manager);
    virtual ~PowerSave();

    static PowerSave* get_instance() { return instance_; };

    static void global_init(PowerWrapperManager* wrapper_manager);

    static void global_deinit() { delete instance_; };

    // 执行节能动作
    bool do_save(PowerAction action, std::string& error);

private:
    void init();

private:
    static PowerSave* instance_;
    PowerWrapperManager* wrapper_manager_;

    PowerSaveComputer save_computer_;
    PowerSaveDpms save_dpms_;
};
}  // namespace Kiran