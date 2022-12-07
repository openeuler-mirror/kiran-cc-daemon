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

#include "plugins/power/backlight/power-backlight-interface.h"

namespace Kiran
{
class PowerBacklightMonitorsTool : public PowerBacklightMonitors
{
public:
    PowerBacklightMonitorsTool();
    virtual ~PowerBacklightMonitorsTool(){};

    static bool support_backlight();

    virtual void init();
    // 获取所有显示器亮度设置对象
    virtual PowerBacklightAbsoluteVec get_monitors() { return this->backlight_monitors_; };
    virtual sigc::signal<void> signal_monitor_changed() { return this->monitor_changed_; };
    virtual sigc::signal<void> signal_brightness_changed() { return this->brightness_changed_; };

private:
    std::string get_backlight_dir();
    void on_brightness_changed(const Glib::RefPtr<Gio::File> &file,
                               const Glib::RefPtr<Gio::File> &other_file,
                               Gio::FileMonitorEvent event_type);

private:
    sigc::signal<void> monitor_changed_;
    sigc::signal<void> brightness_changed_;
    Glib::RefPtr<Gio::FileMonitor> brightness_monitor_;

    PowerBacklightAbsoluteVec backlight_monitors_;
};
}  // namespace Kiran