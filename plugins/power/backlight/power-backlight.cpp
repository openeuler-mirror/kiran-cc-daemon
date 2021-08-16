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

#include "plugins/power/backlight/power-backlight.h"

#include "plugins/power/backlight/power-backlight-kbd.h"
#include "plugins/power/backlight/power-backlight-monitor.h"

namespace Kiran
{
PowerBacklight::PowerBacklight()
{
    this->backlight_monitor_ = std::make_shared<PowerBacklightMonitor>();
    this->backlight_kbd_ = std::make_shared<PowerBacklightKbd>();
}

PowerBacklight::~PowerBacklight()
{
}

PowerBacklight* PowerBacklight::instance_ = nullptr;
void PowerBacklight::global_init()
{
    instance_ = new PowerBacklight();
    instance_->init();
}

std::shared_ptr<PowerBacklightPercentage> PowerBacklight::get_backlight_device(PowerDeviceType device)
{
    switch (device)
    {
    case PowerDeviceType::POWER_DEVICE_TYPE_MONITOR:
        return this->backlight_monitor_;
    case PowerDeviceType::POWER_DEVICE_TYPE_KBD:
        return this->backlight_kbd_;
    default:
        break;
    }
    return nullptr;
}

void PowerBacklight::init()
{
    this->backlight_monitor_->init();
    this->backlight_kbd_->init();

    this->backlight_monitor_->signal_brightness_changed().connect(
        sigc::bind(sigc::mem_fun(this, &PowerBacklight::on_backlight_brightness_changed),
                   this->backlight_monitor_));

    this->backlight_kbd_->signal_brightness_changed().connect(
        sigc::bind(sigc::mem_fun(this, &PowerBacklight::on_backlight_brightness_changed),
                   this->backlight_kbd_));
}

void PowerBacklight::on_backlight_brightness_changed(int32_t brightness_percentage, std::shared_ptr<PowerBacklightPercentage> backlight)
{
    this->brightness_changed_.emit(backlight, brightness_percentage);
}

}  // namespace Kiran