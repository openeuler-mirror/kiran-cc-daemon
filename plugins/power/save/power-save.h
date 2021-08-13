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

private:
    static PowerSave* instance_;
    PowerWrapperManager* wrapper_manager_;

    PowerSaveComputer save_computer_;
    PowerSaveDpms save_dpms_;
};
}  // namespace Kiran