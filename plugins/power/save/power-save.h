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

#include "plugins/power/backlight/power-backlight.h"
#include "plugins/power/save/power-save-computer.h"
#include "plugins/power/save/power-save-dpms.h"
#include "plugins/power/wrapper/power-wrapper-manager.h"
#include "power-i.h"

namespace Kiran
{
class PowerSave
{
public:
    PowerSave(PowerWrapperManager* wrapper_manager, PowerBacklight* backlight);
    virtual ~PowerSave();

    static PowerSave* get_instance() { return instance_; };

    static void global_init(PowerWrapperManager* wrapper_manager, PowerBacklight* backlight);

    static void global_deinit() { delete instance_; };

    // 执行节能动作
    bool do_save(PowerAction action, std::string& error);

    // 显示器是否处于变暗状态
    bool is_display_dimmed();
    // 执行显示器变暗，如果已经处于变暗状态，则返回失败
    bool do_display_dimmed();
    // 执行显示器恢复变暗操作
    void do_display_restore_dimmed();

    // 执行CPU节能操作
    void do_cpu_saver();
    // 执行恢复CPU节能操作
    void do_cpu_restore_saver();

private:
    void init();

    void on_kbd_brightness_changed(int32_t brightness_percentage);
    void on_monitor_brightness_changed(int32_t brightness_percentage);
    void on_active_profile_changed(const Glib::ustring& active_profile);

private:
    static PowerSave* instance_;
    PowerWrapperManager* wrapper_manager_;
    PowerBacklight* backlight_;

    Glib::RefPtr<Gio::Settings> power_settings_;
    std::shared_ptr<PowerBacklightPercentage> backlight_kbd_;
    std::shared_ptr<PowerBacklightPercentage> backlight_monitor_;
    std::shared_ptr<PowerProfiles> profiles_;

    // 记录亮度变暗前的亮度值
    int32_t kbd_restore_brightness_;
    int32_t monitor_restore_brightness_;
    // 记录变暗的时间
    time_t display_dimmed_timestamp_;

    // 记录节能之前的状态
    uint32_t cpu_saver_cookie_;
    time_t cpu_saver_timestamp_;

    PowerSaveComputer save_computer_;
    PowerSaveDpms save_dpms_;
};
}  // namespace Kiran