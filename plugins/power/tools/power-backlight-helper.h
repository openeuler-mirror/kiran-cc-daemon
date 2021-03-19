/**
 * @file          /kiran-cc-daemon/plugins/power/tools/power-backlight-helper.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#pragma once

#include "lib/base/base.h"

namespace Kiran
{
class PowerBacklightHelper
{
public:
    PowerBacklightHelper();
    virtual ~PowerBacklightHelper();

    void init();

    // 是否支持亮度设置
    bool support_backlight() { return (this->brightness_value_ >= 0); };

    // 获取亮度值
    int32_t get_brightness_value();
    // 获取亮度最大值
    int32_t get_brightness_max_value();
    // 设置亮度值
    bool set_brightness_value(int32_t brightness_value, std::string &error);

    // 亮度变化的信号
    sigc::signal<void, int32_t> signal_brightness_changed() { return this->brightness_changed_; };

private:
    std::string get_backlight_filepath();

    void on_brightness_changed(const Glib::RefPtr<Gio::File> &file, const Glib::RefPtr<Gio::File> &other_file, Gio::FileMonitorEvent event_type);

private:
    // 优先搜索的背光配置目录
    const static std::vector<std::string> backlight_search_subdirs_;
    // 背光配置目录
    std::string backlight_dir_;
    Glib::RefPtr<Gio::FileMonitor> brightness_monitor_;
    // 当前亮度值
    int32_t brightness_value_;
    // 亮度变化信号
    sigc::signal<void, int32_t> brightness_changed_;
};
}  // namespace Kiran
