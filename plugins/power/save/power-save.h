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

#include "plugins/power/save/power-save-computer.h"
#include "plugins/power/save/power-save-dpms.h"
#include "plugins/power/wrapper/power-wrapper-manager.h"
#include "power-i.h"

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

    std::string action2str(PowerAction action);

private:
    static PowerSave* instance_;
    PowerWrapperManager* wrapper_manager_;

    PowerSaveComputer save_computer_;
    PowerSaveDpms save_dpms_;
};
}  // namespace Kiran