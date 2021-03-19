/**
 * @file          /kiran-cc-daemon/plugins/power/backlight/power-backlight.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
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