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

#include "plugins/power/tray/power-tray.h"

#include <glib/gi18n.h>
#include "plugins/power/wrapper/power-upower.h"
#include "plugins/power/power-utils.h"
#include "power-i.h"

namespace Kiran
{
#define DEFAULT_ICON_NAME "ksm-ac-adapter-symbolic"

PowerTray::PowerTray()
{
    this->upower_client_ = std::make_shared<PowerUPower>();
    this->upower_settings_ = Gio::Settings::create(POWER_SCHEMA_ID);
    this->status_icon_ = gtk_status_icon_new();
}

PowerTray::~PowerTray()
{
    g_clear_pointer(&this->status_icon_, g_object_unref);
}

PowerTray* PowerTray::instance_ = nullptr;
void PowerTray::global_init()
{
    instance_ = new PowerTray();
    instance_->init();
}

void PowerTray::init()
{
    KLOG_PROFILE("");

    this->upower_client_->init();

    this->update_status_icon();

    this->upower_settings_->signal_changed().connect(sigc::mem_fun(this, &PowerTray::on_settings_changed));
    this->upower_client_->signal_device_props_changed().connect(sigc::mem_fun(this, &PowerTray::on_device_props_changed));
    // 这里需要进行延时处理，因为StatusIcon需要从x11中获取新的前景色，需要等获取到前景色后再进行更新
    Gtk::Settings::get_default()->property_gtk_theme_name().signal_changed().connect(sigc::mem_fun0(this, &PowerTray::delay_update_status_icon));
}

void PowerTray::update_status_icon()
{
    KLOG_PROFILE("");
    std::string icon_name;
    // 托盘图标显示只考虑电源、电池和UPS供电的情况。
    auto icon_policy = PowerTrayIconPolicy(this->upower_settings_->get_enum(POWER_SCHEMA_TRAY_ICON_POLICY));
    auto device_for_tray = this->get_device_for_tray({UP_DEVICE_KIND_BATTERY, UP_DEVICE_KIND_UPS}, icon_name);

    switch (icon_policy)
    {
    case PowerTrayIconPolicy::POWER_TRAY_ICON_POLICY_NERVER:
        icon_name = std::string();
        break;
    case PowerTrayIconPolicy::POWER_TRAY_ICON_POLICY_ALWAYS:
    {
        // 如果没有电池和UPS，则使用电源默认图标
        if (icon_name.empty())
        {
            icon_name = DEFAULT_ICON_NAME;
        }
        break;
    }
    case PowerTrayIconPolicy::POWER_TRAY_ICON_POLICY_PRESENT:
        break;
    default:
        break;
    }

    KLOG_DEBUG("icon name: %s.", icon_name.c_str());

    if (icon_name.empty())
    {
        gtk_status_icon_set_visible(this->status_icon_, false);
    }
    else
    {
        // this->icon_pixbuf_ = this->get_pixbuf_by_icon_name(icon_name);
        // gtk_status_icon_set_from_pixbuf(this->status_icon_, this->icon_pixbuf_->gobj());
        gtk_status_icon_set_from_icon_name(this->status_icon_, icon_name.c_str());
        gtk_status_icon_set_visible(this->status_icon_, true);
    }

    // 对于电源和UPS设备需要显示电量
    this->update_status_icon_toolstip(device_for_tray);
}

void PowerTray::update_status_icon_toolstip(std::shared_ptr<PowerUPowerDevice> device_for_tray)
{
    RETURN_IF_FALSE(device_for_tray);

    switch (device_for_tray->get_props().state)
    {
    case UP_DEVICE_STATE_CHARGING:
    {
        auto time_to_full_text = PowerUtils::get_time_translation(device_for_tray->get_props().time_to_full);
        auto tooltip_text = fmt::format(_("Remaining electricty: {0:.1f}%, approximately {1} until charged"),
                                        device_for_tray->get_props().percentage,
                                        time_to_full_text);
        gtk_status_icon_set_tooltip_text(this->status_icon_, tooltip_text.c_str());
        break;
    }
    case UP_DEVICE_STATE_DISCHARGING:
    {
        auto time_to_empty_text = PowerUtils::get_time_translation(device_for_tray->get_props().time_to_empty);
        auto tooltip_text = fmt::format(_("Remaining electricty: {0:.1f}%, approximately provides {1} runtime"),
                                        device_for_tray->get_props().percentage,
                                        time_to_empty_text);
        gtk_status_icon_set_tooltip_text(this->status_icon_, tooltip_text.c_str());
        break;
    }
    case UP_DEVICE_STATE_FULLY_CHARGED:
    default:
    {
        auto tooltip_text = fmt::format(_("Remaining electricty: {0:.1f}%"), device_for_tray->get_props().percentage);
        gtk_status_icon_set_tooltip_text(this->status_icon_, tooltip_text.c_str());
    }
    break;
    }
}

void PowerTray::delay_update_status_icon()
{
    RETURN_IF_TRUE(this->update_icon_handler_);

    auto timeout = Glib::MainContext::get_default()->signal_timeout();
    this->update_icon_handler_ = timeout.connect([this]() -> bool
                                                 {
                                                     this->update_status_icon();
                                                     return false;
                                                 },
                                                 100);
}

std::shared_ptr<PowerUPowerDevice> PowerTray::get_device_for_tray(const std::vector<uint32_t>& device_types, std::string& icon_name)
{
    for (auto device_type : device_types)
    {
        for (auto upower_device : this->upower_client_->get_devices())
        {
            auto& device_props = upower_device->get_props();
            if (device_props.type == device_type &&
                device_props.is_present)
            {
                icon_name = this->get_device_icon_name(upower_device);
                RETURN_VAL_IF_TRUE(icon_name.length() > 0, upower_device);
            }
        }
    }
    return std::shared_ptr<PowerUPowerDevice>();
}

Glib::RefPtr<Gdk::Pixbuf> PowerTray::get_pixbuf_by_icon_name(const std::string& icon_name)
{
    auto theme_name = Gtk::Settings::get_default()->property_gtk_theme_name().get_value();
    bool was_symbolic = false;
    Gdk::RGBA fg_color;

    if (theme_name.empty())
    {
        KLOG_WARNING("Not found theme name.");
        return Glib::RefPtr<Gdk::Pixbuf>();
    }

    auto provider = Gtk::CssProvider::get_named(theme_name, std::string());
    if (!provider)
    {
        KLOG_WARNING("Not found provider for %s.", theme_name.c_str());
        return Glib::RefPtr<Gdk::Pixbuf>();
    }

    KLOG_DEBUG("Theme name: %s.", theme_name.c_str());

    auto style_context = Gtk::StyleContext::create();
    style_context->add_provider(provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
    auto scale = Gdk::Window::get_default_root_window()->get_scale_factor();
    auto icon_info = Gtk::IconTheme::get_default()->lookup_icon(icon_name, 16, scale);
    return icon_info.load_symbolic(style_context, was_symbolic);
}

std::string PowerTray::get_device_icon_name(std::shared_ptr<PowerUPowerDevice> upower_device)
{
    RETURN_VAL_IF_FALSE(upower_device, std::string());
    // 这里用于兼容使用混合电池的情况
    if (upower_device->get_props().type == UP_DEVICE_KIND_BATTERY)
    {
        upower_device = this->upower_client_->get_display_device();
    }
    RETURN_VAL_IF_FALSE(upower_device, std::string());

    auto& device_props = upower_device->get_props();
    auto kind_str = upower_device->get_kind_string();

    switch (device_props.type)
    {
    case UP_DEVICE_KIND_LINE_POWER:
    case UP_DEVICE_KIND_MONITOR:
    case UP_DEVICE_KIND_MOUSE:
    case UP_DEVICE_KIND_KEYBOARD:
    case UP_DEVICE_KIND_PHONE:
        return DEFAULT_ICON_NAME;
    case UP_DEVICE_KIND_UPS:
    case UP_DEVICE_KIND_BATTERY:
    {
        if (!device_props.is_present)
        {
            // TODO: 暂时用默认图标，后面需要改
            // return fmt::format("ksm-{0}-missing", kind_str);
            return DEFAULT_ICON_NAME;
        }
        auto percentage = this->percentage2index(device_props.percentage);
        switch (device_props.state)
        {
        case UP_DEVICE_STATE_EMPTY:
            return fmt::format("ksm-battery-000");
        case UP_DEVICE_STATE_FULLY_CHARGED:
        case UP_DEVICE_STATE_CHARGING:
        case UP_DEVICE_STATE_PENDING_CHARGE:
            return fmt::format("ksm-battery-{0}-charging-symbolic", percentage);
        case UP_DEVICE_STATE_DISCHARGING:
        case UP_DEVICE_STATE_PENDING_DISCHARGE:
            return fmt::format("ksm-battery-{0}{1}", percentage, percentage != "000" ? "-symbolic" : "");
        default:
            // TODO: 暂时用默认图标，后面需要改
            // return fmt::format("ksm-battery-missing", kind_str);
            return DEFAULT_ICON_NAME;
        }
        break;
    }
    default:
        break;
    }

    return std::string();
}

std::string PowerTray::percentage2index(int32_t percentage)
{
    RETURN_VAL_IF_TRUE(percentage <= 10, "000");
    RETURN_VAL_IF_TRUE(percentage <= 30, "020");
    RETURN_VAL_IF_TRUE(percentage <= 50, "040");
    RETURN_VAL_IF_TRUE(percentage <= 70, "060");
    RETURN_VAL_IF_TRUE(percentage <= 90, "080");
    return "100";
}

void PowerTray::on_settings_changed(const Glib::ustring& key)
{
    switch (shash(key.c_str()))
    {
    case CONNECT(POWER_SCHEMA_TRAY_ICON_POLICY, _hash):
        this->update_status_icon();
        break;
    default:
        break;
    }
}

void PowerTray::on_device_props_changed(std::shared_ptr<PowerUPowerDevice> upwer_device,
                                        const UPowerDeviceProps& old_props,
                                        const UPowerDeviceProps& new_props)
{
    if (old_props.is_present != new_props.is_present ||
        old_props.percentage != new_props.percentage ||
        old_props.state != new_props.state)
    {
        this->update_status_icon();
    }
}
}  // namespace Kiran