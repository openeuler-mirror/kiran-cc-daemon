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
    std::string get_backlight_dir() { return this->backlight_dir_; };

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
