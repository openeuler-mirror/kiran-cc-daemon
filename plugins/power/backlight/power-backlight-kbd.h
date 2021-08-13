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

namespace Kiran
{
// 键盘亮度控制
class PowerBacklightKbd : public PowerBacklightPercentage
{
public:
    PowerBacklightKbd();
    virtual ~PowerBacklightKbd();

    virtual void init() override;

    virtual PowerDeviceType get_type() override { return PowerDeviceType::POWER_DEVICE_TYPE_KBD; };

    // 设置亮度百分比
    virtual bool set_brightness(int32_t percentage) override;
    // 获取亮度百分比，如果小于0，则说明不支持调节亮度
    virtual int32_t get_brightness() override { return this->brightness_percentage_; };

    // 增加亮度
    virtual bool brightness_up() override;
    // 降低亮度
    virtual bool brightness_down() override;

    // 键盘亮度发生变化
    virtual sigc::signal<void, int32_t>& signal_brightness_changed() override { return this->brightness_changed_; };

private:
    // 获取亮度值
    int32_t get_brightness_value();
    // 设置亮度值
    bool set_brightness_value(int32_t value);
    // 获取最大亮度值
    int32_t get_max_brightness_value();

    int32_t brightness_percent2discrete(int32_t percentage, int32_t levels);
    int32_t brightness_discrete2percent(int32_t discrete, int32_t levels);

    void on_upower_kbd_signal(const Glib::ustring& sender_name,
                              const Glib::ustring& signal_name,
                              const Glib::VariantContainerBase& parameters);

private:
    Glib::RefPtr<Gio::DBus::Proxy> upower_kbd_proxy_;

    int32_t brightness_value_;
    int32_t brightness_percentage_;
    int32_t max_brightness_value_;

    sigc::signal<void, int32_t> brightness_changed_;
};
}  // namespace Kiran