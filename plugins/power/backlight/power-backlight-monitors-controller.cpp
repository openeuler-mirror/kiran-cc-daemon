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

#include "plugins/power/backlight/power-backlight-monitors-controller.h"
#include "plugins/power/backlight/power-backlight-monitors-tool.h"
#include "plugins/power/backlight/power-backlight-monitors-x11.h"

namespace Kiran
{
PowerBacklightMonitorsController::PowerBacklightMonitorsController() : brightness_percentage_(-1)
{
    this->power_settings_ = Gio::Settings::create(POWER_SCHEMA_ID);
}

PowerBacklightMonitorsController::~PowerBacklightMonitorsController()
{
}

void PowerBacklightMonitorsController::init()
{
    this->load_backlight_monitors();
    this->brightness_percentage_ = this->get_brightness();
}

bool PowerBacklightMonitorsController::set_brightness(int32_t percentage)
{
    auto monitors = this->backlight_monitors_->get_monitors();
    for (auto &monitor : monitors)
    {
        RETURN_VAL_IF_FALSE(this->set_brightness_percentage(monitor, percentage), false);
    }
    this->update_cached_brightness();
    return true;
}

int32_t PowerBacklightMonitorsController::get_brightness()
{
    auto monitors = this->backlight_monitors_->get_monitors();
    for (auto &monitor : monitors)
    {
        auto percentage = this->get_brightness_percentage(monitor);
        RETURN_VAL_IF_TRUE(percentage >= 0, percentage);
    }
    return -1;
}

bool PowerBacklightMonitorsController::brightness_up()
{
    auto monitors = this->backlight_monitors_->get_monitors();
    for (auto &monitor : monitors)
    {
        this->brightness_value_up(monitor);
    }
    return true;
}

bool PowerBacklightMonitorsController::brightness_down()
{
    auto monitors = this->backlight_monitors_->get_monitors();
    for (auto &monitor : monitors)
    {
        this->brightness_value_down(monitor);
    }
    return true;
}

void PowerBacklightMonitorsController::load_backlight_monitors()
{
    auto monitor_backlight_policy = this->power_settings_->get_enum(POWER_SCHEMA_MONITOR_BACKLIGHT_POLICY);

    switch (monitor_backlight_policy)
    {
    case PowerMonitorBacklightPolicy::POWER_MONITOR_BACKLIGHT_POLICY_TOOL:
        this->backlight_monitors_ = std::make_shared<PowerBacklightMonitorsTool>();
        break;
    case PowerMonitorBacklightPolicy::POWER_MONITOR_BACKLIGHT_POLICY_X11:
        this->backlight_monitors_ = std::make_shared<PowerBacklightMonitorsX11>();
        break;
    default:
    {
        if (PowerBacklightMonitorsTool::support_backlight())
        {
            this->backlight_monitors_ = std::make_shared<PowerBacklightMonitorsTool>();
        }
        else
        {
            this->backlight_monitors_ = std::make_shared<PowerBacklightMonitorsX11>();
        }
    }
    }

    this->backlight_monitors_->init();
    this->backlight_monitors_->signal_monitor_changed().connect(sigc::mem_fun(this, &PowerBacklightMonitorsController::on_monitor_changed));
    this->backlight_monitors_->signal_brightness_changed().connect(sigc::mem_fun(this, &PowerBacklightMonitorsController::update_cached_brightness));
}

bool PowerBacklightMonitorsController::set_brightness_percentage(std::shared_ptr<PowerBacklightAbsolute> absolute_monitor, int32_t percentage)
{
    int32_t brightness_min = -1;
    int32_t brightness_max = -1;

    auto brightness_current_value = absolute_monitor->get_brightness_value();
    RETURN_VAL_IF_TRUE(brightness_current_value < 0, false);
    RETURN_VAL_IF_FALSE(absolute_monitor->get_brightness_range(brightness_min, brightness_max), false);
    RETURN_VAL_IF_TRUE(brightness_max == brightness_min, false);

    auto brightness_set_value = this->brightness_percent2discrete(percentage, (brightness_max - brightness_min) + 1);
    KLOG_DEBUG("min value: %d, max value: %d, current value: %d, set value: %d, set value percent: %d",
               brightness_min,
               brightness_max,
               brightness_current_value,
               brightness_set_value,
               percentage);

    brightness_set_value = std::max(brightness_set_value, brightness_min);
    brightness_set_value = std::min(brightness_set_value, brightness_max);

    if (brightness_current_value == brightness_set_value)
    {
        KLOG_DEBUG("The set brightness value is equal to current value.");
        return true;
    }

    // 一些背光控制器的亮度增加和减少是按照一定倍数进行的，例如macbook pro是每次增加5%的亮度值
    auto step = this->get_brightness_step(std::abs(brightness_set_value - brightness_current_value));
    KLOG_DEBUG("Using step of %d", step);

    if (brightness_current_value < brightness_set_value)
    {
        for (int32_t i = brightness_current_value; i <= brightness_set_value; i += step)
        {
            if (!absolute_monitor->set_brightness_value(i))
            {
                break;
            }
            if (i + step <= brightness_set_value)
            {
                g_usleep(5000);
            }
        }
    }
    else
    {
        for (int32_t i = brightness_current_value; i >= brightness_set_value; i -= step)
        {
            if (!absolute_monitor->set_brightness_value(i))
            {
                break;
            }
            if (i - step >= brightness_set_value)
            {
                g_usleep(5000);
            }
        }
    }
    return true;
}

int32_t PowerBacklightMonitorsController::get_brightness_percentage(std::shared_ptr<PowerBacklightAbsolute> absolute_monitor)
{
    int32_t brightness_min = -1;
    int32_t brightness_max = -1;

    auto brightness_value = absolute_monitor->get_brightness_value();
    RETURN_VAL_IF_TRUE(brightness_value < 0, -1);
    RETURN_VAL_IF_FALSE(absolute_monitor->get_brightness_range(brightness_min, brightness_max), -1);
    RETURN_VAL_IF_TRUE(brightness_min >= brightness_max, -1);

    KLOG_DEBUG("output brightness info: value %d, min %d, max %d",
               brightness_value,
               brightness_min,
               brightness_max);

    int32_t brightness_level = brightness_max - brightness_min + 1;
    auto percentage = this->brightness_discrete2percent(brightness_value, brightness_level);
    KLOG_DEBUG("percentage %i", percentage);
    return percentage;
}

bool PowerBacklightMonitorsController::brightness_value_up(std::shared_ptr<PowerBacklightAbsolute> absolute_monitor)
{
    int32_t brightness_min = -1;
    int32_t brightness_max = -1;

    auto brightness_current_value = absolute_monitor->get_brightness_value();
    RETURN_VAL_IF_TRUE(brightness_current_value < 0, false);
    RETURN_VAL_IF_FALSE(absolute_monitor->get_brightness_range(brightness_min, brightness_max), false);
    RETURN_VAL_IF_TRUE(brightness_max == brightness_min, false);

    RETURN_VAL_IF_TRUE(brightness_current_value == brightness_max, true);

    brightness_current_value += this->get_brightness_step((brightness_max - brightness_min) + 1);
    brightness_current_value = std::min(brightness_current_value, brightness_max);
    return absolute_monitor->set_brightness_value(brightness_current_value);
}

bool PowerBacklightMonitorsController::brightness_value_down(std::shared_ptr<PowerBacklightAbsolute> absolute_monitor)
{
    int32_t brightness_min = -1;
    int32_t brightness_max = -1;

    auto brightness_current_value = absolute_monitor->get_brightness_value();
    RETURN_VAL_IF_TRUE(brightness_current_value < 0, false);
    RETURN_VAL_IF_FALSE(absolute_monitor->get_brightness_range(brightness_min, brightness_max), false);
    RETURN_VAL_IF_TRUE(brightness_max == brightness_min, false);

    RETURN_VAL_IF_TRUE(brightness_current_value == brightness_min, true);

    brightness_current_value -= this->get_brightness_step((brightness_max - brightness_min) + 1);
    brightness_current_value = std::max(brightness_current_value, brightness_min);
    return absolute_monitor->set_brightness_value(brightness_current_value);
}

int32_t PowerBacklightMonitorsController::brightness_discrete2percent(int32_t discrete, int32_t levels)
{
    RETURN_VAL_IF_TRUE(discrete > levels, 100);
    RETURN_VAL_IF_TRUE(levels <= 1, 0);
    return (int32_t)(((double)discrete * (100.0 / (double)(levels - 1))) + 0.5);
}

int32_t PowerBacklightMonitorsController::brightness_percent2discrete(int32_t percentage, int32_t levels)
{
    RETURN_VAL_IF_TRUE(percentage > 100, levels);
    RETURN_VAL_IF_TRUE(levels == 0, 0);

    return (int32_t)((((double)percentage * (double)(levels - 1)) / 100.0) + 0.5);
}

int32_t PowerBacklightMonitorsController::get_brightness_step(uint32_t levels)
{
    if (levels > 20)
    {
        return levels / 20;
    }
    return 1;
}

void PowerBacklightMonitorsController::update_cached_brightness()
{
    auto brightness_percentage = this->get_brightness();
    if (brightness_percentage != this->brightness_percentage_)
    {
        this->brightness_percentage_ = brightness_percentage;
        this->brightness_changed_.emit(this->brightness_percentage_);
    }
}

void PowerBacklightMonitorsController::on_monitor_changed()
{
    this->update_cached_brightness();
}

}  // namespace Kiran