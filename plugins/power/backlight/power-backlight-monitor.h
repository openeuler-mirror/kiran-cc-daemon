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

#include <gdkmm.h>
//
#include <X11/Xatom.h>

#include "plugins/power/backlight/power-backlight-base.h"
#include "plugins/power/backlight/power-backlight-x11.h"
#include "plugins/power/tools/power-backlight-helper.h"

namespace Kiran
{
/* 显示器亮度控制。显示器亮度设置适用于笔记本电脑和带背光控制器的显示器，
   台式机的显示器不一定带有背光控制器，因此可能无法通过该模块的接口调节亮度，
   台式机的显示器一般可以直接通过显示器周边的按钮调节亮度 */

class PowerBacklightMonitor : public PowerBacklightPercentage
{
public:
    PowerBacklightMonitor();
    virtual ~PowerBacklightMonitor();

    virtual void init();

    virtual PowerDeviceType get_type() override { return PowerDeviceType::POWER_DEVICE_TYPE_MONITOR; };

    // 设置亮度百分比，如果不支持设置或者不存在可设置的显示器则返回false
    virtual bool set_brightness(int32_t percentage) override;
    /* 获取亮度百分比
       如果存在多个显示器有不同的亮度值，只会取其中的一个进行返回
       如果返回负数则表示获取失败
    */
    virtual int32_t get_brightness() override;

    // 增加亮度
    bool brightness_up();
    // 降低亮度
    bool brightness_down();

    // 显示器亮度发生变化
    virtual sigc::signal<void, int32_t> &signal_brightness_changed() override { return this->brightness_changed_; };

private:
    void load_absolute_monitors();
    // 设置单个显示器的亮度百分比
    bool set_brightness_percentage(std::shared_ptr<PowerBacklightAbsolute> absolute_monitor, int32_t percentage);
    // 获取单个显示器的亮度
    int32_t get_brightness_percentage(std::shared_ptr<PowerBacklightAbsolute> absolute_monitor);
    // 增加单个显示器亮度
    bool brightness_value_up(std::shared_ptr<PowerBacklightAbsolute> absolute_monitor);
    // 降低单个显示器亮度
    bool brightness_value_down(std::shared_ptr<PowerBacklightAbsolute> absolute_monitor);

    int32_t brightness_discrete2percent(int32_t discrete, int32_t levels);
    int32_t brightness_percent2discrete(int32_t percentage, int32_t levels);
    int32_t get_brightness_step(uint32_t levels);

    // 更新缓存的亮度百分比并发送信号
    void update_cached_brightness();

    void on_x11_monitor_changed(PBXMonitorEvent x11_monitor_event);
    void on_helper_brightness_changed(int32_t brightness_value);

private:
    PowerBacklightX11 backlight_x11_;
    PowerBacklightHelper backlight_helper_;

    // 用于调节显示器的绝对值
    PowerBacklightAbsoluteVec absolute_monitors_;

    int32_t brightness_percentage_;
    sigc::signal<void, int32_t> brightness_changed_;
};
}  // namespace Kiran