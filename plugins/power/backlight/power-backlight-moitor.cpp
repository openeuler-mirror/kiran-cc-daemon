/**
 * @file          /kiran-cc-daemon/plugins/power/backlight/power-backlight-moitor.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#include "plugins/power/backlight/power-backlight-monitor-tool.h"
#include "plugins/power/backlight/power-backlight-monitor.h"

namespace Kiran
{
PowerBacklightMonitor::PowerBacklightMonitor() : brightness_percentage_(-1)
{
}

PowerBacklightMonitor::~PowerBacklightMonitor()
{
}

void PowerBacklightMonitor::init()
{
    SETTINGS_PROFILE("");

    backlight_x11_.init();
    this->load_absolute_monitors();
    this->brightness_percentage_ = this->get_brightness();

    this->backlight_x11_.signal_monitor_changed().connect(sigc::mem_fun(this, &PowerBacklightMonitor::on_x11_monitor_changed));
}

bool PowerBacklightMonitor::set_brightness(int32_t percentage)
{
    RETURN_VAL_IF_TRUE(this->absolute_monitors_.size() == 0, false);

    for (auto &monitor : this->absolute_monitors_)
    {
        RETURN_VAL_IF_FALSE(this->set_brightness_percentage(monitor, percentage), false);
    }
    return true;
}

int32_t PowerBacklightMonitor::get_brightness()
{
    for (auto &monitor : this->absolute_monitors_)
    {
        auto percentage = this->get_brightness_percentage(monitor);
        RETURN_VAL_IF_TRUE(percentage >= 0, percentage);
    }
    return -1;
}

bool PowerBacklightMonitor::brightness_up()
{
    RETURN_VAL_IF_TRUE(this->absolute_monitors_.size() == 0, false);

    for (auto &monitor : this->absolute_monitors_)
    {
        this->brightness_value_up(monitor);
    }
    return true;
}

bool PowerBacklightMonitor::brightness_down()
{
    RETURN_VAL_IF_TRUE(this->absolute_monitors_.size() == 0, false);
    for (auto &monitor : this->absolute_monitors_)
    {
        this->brightness_value_down(monitor);
    }
    return true;
}

void PowerBacklightMonitor::load_absolute_monitors()
{
    this->absolute_monitors_.clear();

    if (this->backlight_x11_.support_backlight_extension())
    {
        auto monitors = this->backlight_x11_.get_monitors();
        this->absolute_monitors_ = PowerBacklightAbsoluteVec(monitors.begin(), monitors.end());
    }
    else
    {
        this->absolute_monitors_.push_back(std::make_shared<PowerBacklightMonitorTool>());
    }
}

bool PowerBacklightMonitor::set_brightness_percentage(std::shared_ptr<PowerBacklightAbsolute> absolute_monitor, int32_t percentage)
{
    int32_t brightness_min = -1;
    int32_t brightness_max = -1;

    auto brightness_current_value = absolute_monitor->get_brightness_value();
    RETURN_VAL_IF_TRUE(brightness_current_value < 0, false);
    RETURN_VAL_IF_FALSE(absolute_monitor->get_brightness_range(brightness_min, brightness_max), false);
    RETURN_VAL_IF_TRUE(brightness_max == brightness_min, false);

    auto brightness_set_value = this->brightness_percent2discrete(percentage, (brightness_max - brightness_min) + 1);
    LOG_DEBUG("min value: %d, max value: %d, current value: %d, set value: %d, set value percent: %d",
              brightness_min,
              brightness_max,
              brightness_current_value,
              brightness_set_value,
              percentage);

    brightness_set_value = std::max(brightness_set_value, brightness_min);
    brightness_set_value = std::min(brightness_set_value, brightness_max);

    if (brightness_current_value == brightness_set_value)
    {
        LOG_DEBUG("The set brightness value is equal to current value.");
        return true;
    }

    // 一些背光控制器的亮度增加和减少是按照一定倍数进行的，例如macbook pro是每次增加5%的亮度值
    auto step = this->get_brightness_step(std::abs(brightness_set_value - brightness_current_value));
    LOG_DEBUG("Using step of %d", step);

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

int32_t PowerBacklightMonitor::get_brightness_percentage(std::shared_ptr<PowerBacklightAbsolute> absolute_monitor)
{
    int32_t brightness_min = -1;
    int32_t brightness_max = -1;

    auto brightness_value = absolute_monitor->get_brightness_value();
    RETURN_VAL_IF_TRUE(brightness_value < 0, -1);
    RETURN_VAL_IF_FALSE(absolute_monitor->get_brightness_range(brightness_min, brightness_max), -1);
    RETURN_VAL_IF_TRUE(brightness_min >= brightness_max, -1);

    LOG_DEBUG("output brightness info: value %d, min %d, max %d",
              brightness_value,
              brightness_min,
              brightness_max);

    int32_t brightness_level = brightness_max - brightness_min + 1;
    auto percentage = this->brightness_discrete2percent(brightness_value, brightness_level);
    LOG_DEBUG("percentage %i", percentage);
    return percentage;
}

bool PowerBacklightMonitor::brightness_value_up(std::shared_ptr<PowerBacklightAbsolute> absolute_monitor)
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

bool PowerBacklightMonitor::brightness_value_down(std::shared_ptr<PowerBacklightAbsolute> absolute_monitor)
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

int32_t PowerBacklightMonitor::brightness_discrete2percent(int32_t discrete, int32_t levels)
{
    // TODO: test
    RETURN_VAL_IF_TRUE(discrete > levels, 100);
    RETURN_VAL_IF_TRUE(levels <= 1, 0);
    return (int32_t)(((double)discrete * (100.0 / (double)(levels - 1))) + 0.5);
}

int32_t PowerBacklightMonitor::brightness_percent2discrete(int32_t percentage, int32_t levels)
{
    RETURN_VAL_IF_TRUE(percentage > 100, levels);
    RETURN_VAL_IF_TRUE(levels == 0, 0);

    return (int32_t)((((double)percentage * (double)(levels - 1)) / 100.0) + 0.5);
}

int32_t PowerBacklightMonitor::get_brightness_step(uint32_t levels)
{
    if (levels > 20)
    {
        return levels / 20;
    }
    return 1;
}

void PowerBacklightMonitor::on_x11_monitor_changed(PBXMonitorEvent x11_monitor_event)
{
    switch (x11_monitor_event)
    {
    case PBXMonitorEvent::PBX_MONITOR_EVENT_PROPERTY_CHANGED:
    {
        auto brightness_percentage = this->get_brightness();
        if (brightness_percentage != this->brightness_percentage_)
        {
            this->brightness_percentage_ = brightness_percentage;
            this->brightness_changed_.emit(this->brightness_percentage_);
        }
        break;
    }
    case PBXMonitorEvent::PBX_MONITOR_EVENT_SCREEN_CHANGED:
        this->load_absolute_monitors();
        break;
    default:
        break;
    }
}

}  // namespace Kiran