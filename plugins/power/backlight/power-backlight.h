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