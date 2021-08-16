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

#include "plugins/power/backlight/power-backlight-kbd.h"

namespace Kiran
{
#define UPOWER_KBD_BACKLIGHT_DBUS_NAME "org.freedesktop.UPower"
#define UPOWER_KBD_BACKLIGHT_DBUS_OBJECT "/org/freedesktop/UPower/KbdBacklight"
#define UPOWER_KBD_BACKLIGHT_DBUS_INTERFACE "org.freedesktop.UPower.KbdBacklight"

#define POWER_KBD_BACKLIGHT_STEP 10

PowerBacklightKbd::PowerBacklightKbd() : brightness_value_(-1),
                                         brightness_percentage_(-1),
                                         max_brightness_value_(-1)
{
}

PowerBacklightKbd::~PowerBacklightKbd()
{
}

void PowerBacklightKbd::init()
{
    try
    {
        this->upower_kbd_proxy_ = Gio::DBus::Proxy::create_for_bus_sync(Gio::DBus::BUS_TYPE_SYSTEM,
                                                                        UPOWER_KBD_BACKLIGHT_DBUS_NAME,
                                                                        UPOWER_KBD_BACKLIGHT_DBUS_OBJECT,
                                                                        UPOWER_KBD_BACKLIGHT_DBUS_INTERFACE);
    }
    catch (const Glib::Error& e)
    {
        KLOG_DEBUG("%s", e.what().c_str());
        return;
    }

    this->max_brightness_value_ = this->get_max_brightness_value();
    // 判断是否支持亮度设置
    RETURN_IF_TRUE(this->max_brightness_value_ <= 1);

    this->brightness_value_ = this->get_brightness_value();
    this->brightness_percentage_ = this->brightness_discrete2percent(this->brightness_value_, this->max_brightness_value_);

    this->upower_kbd_proxy_->signal_signal().connect(sigc::mem_fun(this, &PowerBacklightKbd::on_upower_kbd_signal));
}

bool PowerBacklightKbd::set_brightness(int32_t percentage)
{
    RETURN_VAL_IF_TRUE(this->max_brightness_value_ <= 1, false);
    RETURN_VAL_IF_TRUE(percentage == this->brightness_percentage_, true);

    auto new_brightness_value = this->brightness_percent2discrete(percentage, this->max_brightness_value_);
    auto adjust_scale = (percentage > this->brightness_percentage_) ? 1 : -1;

    // 如果设置的百分比和当前的百分比对应的值是相同的，则向上或者向下调整亮度值
    if (new_brightness_value == this->brightness_value_)
    {
        new_brightness_value += adjust_scale;
    }

    while (this->brightness_value_ != new_brightness_value)
    {
        this->brightness_value_ += adjust_scale;
        if (!this->set_brightness_value(this->brightness_value_))
        {
            break;
        }
    }
    this->brightness_percentage_ = this->brightness_discrete2percent(this->brightness_value_, this->max_brightness_value_);
    KLOG_DEBUG("current: %d, new: %d.", this->brightness_value_, new_brightness_value);

    return (this->brightness_value_ == new_brightness_value);
}

bool PowerBacklightKbd::brightness_up()
{
    RETURN_VAL_IF_TRUE(this->max_brightness_value_ <= 1, false);

    auto brightness_percentage = std::min(this->brightness_percentage_ + POWER_KBD_BACKLIGHT_STEP, 100);
    return this->set_brightness(brightness_percentage);
}

bool PowerBacklightKbd::brightness_down()
{
    RETURN_VAL_IF_TRUE(this->max_brightness_value_ <= 1, false);

    auto brightness_percentage = std::max(this->brightness_percentage_ - POWER_KBD_BACKLIGHT_STEP, 0);
    return this->set_brightness(brightness_percentage);
}

int32_t PowerBacklightKbd::get_brightness_value()
{
    RETURN_VAL_IF_FALSE(this->upower_kbd_proxy_, -1);

    try
    {
        auto retval = this->upower_kbd_proxy_->call_sync("GetBrightness", Glib::VariantContainerBase());
        auto v1 = retval.get_child(0);
        auto brightness_value = Glib::VariantBase::cast_dynamic<Glib::Variant<int32_t>>(v1).get();
        return brightness_value;
    }
    catch (const Glib::Error& e)
    {
        KLOG_DEBUG("%s", e.what().c_str());
    }
    catch (const std::exception& e)
    {
        KLOG_WARNING("%s", e.what());
    }
    return -1;
}

bool PowerBacklightKbd::set_brightness_value(int32_t value)
{
    RETURN_VAL_IF_FALSE(this->upower_kbd_proxy_, false);

    auto parameters = g_variant_new("(i)", value);
    Glib::VariantContainerBase base(parameters, false);

    try
    {
        this->upower_kbd_proxy_->call_sync("SetBrightness", base);
    }
    catch (const Glib::Error& e)
    {
        KLOG_DEBUG("%s", e.what().c_str());
        return false;
    }
    return true;
}

int32_t PowerBacklightKbd::get_max_brightness_value()
{
    RETURN_VAL_IF_FALSE(this->upower_kbd_proxy_, -1);

    try
    {
        auto retval = this->upower_kbd_proxy_->call_sync("GetMaxBrightness", Glib::VariantContainerBase());
        auto v1 = retval.get_child(0);
        auto max_brightness_value = Glib::VariantBase::cast_dynamic<Glib::Variant<int32_t>>(v1).get();
        return max_brightness_value;
    }
    catch (const Glib::Error& e)
    {
        KLOG_DEBUG("%s", e.what().c_str());
    }
    catch (const std::exception& e)
    {
        KLOG_WARNING("%s", e.what());
    }
    return -1;
}

int32_t PowerBacklightKbd::brightness_percent2discrete(int32_t percentage, int32_t levels)
{
    RETURN_VAL_IF_TRUE(percentage > 100, levels);
    RETURN_VAL_IF_TRUE(levels == 0, 0);

    /* for levels < 10 min value is 0 */
    int32_t factor = levels < 10 ? 0 : 1;
    return (int32_t)((((double)percentage * (double)(levels - factor)) / 100.0f) + 0.5f);
}

int32_t PowerBacklightKbd::brightness_discrete2percent(int32_t discrete, int32_t levels)
{
    RETURN_VAL_IF_TRUE(discrete > levels, 100);
    RETURN_VAL_IF_TRUE(levels == 0, 0);

    /* for levels < 10 min value is 0 */
    int32_t factor = levels < 10 ? 0 : 1;
    return (int32_t)(((double)discrete * (100.0f / (double)(levels - factor))) + 0.5f);
}

void PowerBacklightKbd::on_upower_kbd_signal(const Glib::ustring& sender_name,
                                             const Glib::ustring& signal_name,
                                             const Glib::VariantContainerBase& parameters)
{
    KLOG_PROFILE("sender_name: %s, signal_name: %s.", sender_name.c_str(), signal_name.c_str());

    switch (shash(signal_name.c_str()))
    {
    case "BrightnessChanged"_hash:
    {
        try
        {
            Glib::VariantContainerBase v1;
            parameters.get_child(v1, 0);
            this->brightness_value_ = Glib::VariantBase::cast_dynamic<Glib::Variant<int32_t>>(v1).get();
            this->brightness_percentage_ = this->brightness_discrete2percent(this->brightness_value_, this->max_brightness_value_);
            this->brightness_changed_.emit(this->brightness_percentage_);
        }
        catch (const std::exception& e)
        {
            KLOG_WARNING("%s.", e.what());
        }
        break;
    }
    default:
        break;
    }
}

}  // namespace  Kiran
