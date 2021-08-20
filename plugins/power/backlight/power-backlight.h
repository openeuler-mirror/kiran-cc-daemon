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

#include "plugins/power/backlight/power-backlight-base.h"
#include "power-i.h"


namespace Kiran
{
// 背光设备的亮度控制管理
class PowerBacklight
{
public:
    PowerBacklight();
    virtual ~PowerBacklight();

    static PowerBacklight* get_instance() { return instance_; };

    static void global_init();

    static void global_deinit() { delete instance_; };

    std::shared_ptr<PowerBacklightPercentage> get_backlight_device(PowerDeviceType device);

    // 背光设备亮度发生变化
    sigc::signal<void, std::shared_ptr<PowerBacklightPercentage>, int32_t>& signal_brightness_changed() { return this->brightness_changed_; };

private:
    void init();

    void on_backlight_brightness_changed(int32_t brightness_percentage, std::shared_ptr<PowerBacklightPercentage> backlight);

private:
    static PowerBacklight* instance_;

    std::shared_ptr<PowerBacklightPercentage> backlight_kbd_;
    std::shared_ptr<PowerBacklightPercentage> backlight_monitor_;

    sigc::signal<void, std::shared_ptr<PowerBacklightPercentage>, int32_t> brightness_changed_;
};
}  // namespace Kiran