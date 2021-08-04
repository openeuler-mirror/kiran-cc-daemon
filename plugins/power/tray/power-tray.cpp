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

#include "plugins/power/tray/power-tray.h"

#include "power-i.h"

namespace Kiran
{
#define DEFAULT_ICON_NAME "gpm-ac-adapter"

PowerTray::PowerTray(PowerWrapperManager* wrapper_manager) : wrapper_manager_(wrapper_manager)
{
    this->upower_client_ = wrapper_manager_->get_default_upower();
    this->upower_settings_ = Gio::Settings::create(POWER_SCHEMA_ID);
    this->status_icon_ = gtk_status_icon_new();
}

PowerTray::~PowerTray()
{
    g_clear_pointer(&this->status_icon_, g_object_unref);
}

PowerTray* PowerTray::instance_ = nullptr;
void PowerTray::global_init(PowerWrapperManager* wrapper_manager)
{
    instance_ = new PowerTray(wrapper_manager);
    instance_->init();
}

void PowerTray::init()
{
    this->update_status_icon();

    this->upower_settings_->signal_changed().connect(sigc::mem_fun(this, &PowerTray::on_settings_changed));
    this->upower_client_->signal_device_props_changed().connect(sigc::mem_fun(this, &PowerTray::on_device_props_changed));
}

void PowerTray::update_status_icon()
{
    // 托盘图标显示只考虑电源、电池和UPS供电的情况。
    auto icon_policy = PowerTrayIconPolicy(this->upower_settings_->get_enum(POWER_SCHEMA_TRAY_ICON_POLICY));
    auto icon_name = this->get_icon_name({UP_DEVICE_KIND_BATTERY, UP_DEVICE_KIND_UPS});

    KLOG_DEBUG("icon name: %s.", icon_name.c_str());

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

    if (icon_name.empty())
    {
        gtk_status_icon_set_visible(this->status_icon_, false);
    }
    else
    {
        gtk_status_icon_set_from_icon_name(this->status_icon_, icon_name.c_str());
        gtk_status_icon_set_visible(this->status_icon_, true);
    }
}

std::string PowerTray::get_icon_name(const std::vector<uint32_t>& device_types)
{
    for (auto device_type : device_types)
    {
        for (auto upower_device : this->upower_client_->get_devices())
        {
            auto& device_props = upower_device->get_props();
            if (device_props.type == device_type &&
                device_props.is_present)
            {
                auto icon_name = this->get_device_icon_name(upower_device);
                RETURN_VAL_IF_TRUE(icon_name.length() > 0, icon_name);
            }
        }
    }
    return std::string();
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
        return "gpm-ac-adapter";
    case UP_DEVICE_KIND_MONITOR:
        return "gpm-monitor";
    case UP_DEVICE_KIND_UPS:
    case UP_DEVICE_KIND_BATTERY:
    case UP_DEVICE_KIND_MOUSE:
    case UP_DEVICE_KIND_KEYBOARD:
    case UP_DEVICE_KIND_PHONE:
    {
        if (!device_props.is_present)
        {
            return fmt::format("gpm-{0}-missing", kind_str);
        }
        switch (device_props.state)
        {
        case UP_DEVICE_STATE_EMPTY:
            return fmt::format("gpm-{0}-000", kind_str);
        case UP_DEVICE_STATE_FULLY_CHARGED:
        case UP_DEVICE_STATE_CHARGING:
        case UP_DEVICE_STATE_PENDING_CHARGE:
            return fmt::format("gpm-{0}-{1}-charging", kind_str, this->percentage2index(device_props.percentage));
        case UP_DEVICE_STATE_DISCHARGING:
        case UP_DEVICE_STATE_PENDING_DISCHARGE:
            return fmt::format("gpm-{0}-{1}", kind_str, this->percentage2index(device_props.percentage));
        default:
            return fmt::format("gpm-{0}-missing", kind_str);
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
    RETURN_VAL_IF_TRUE(percentage < 10, "000");
    RETURN_VAL_IF_TRUE(percentage < 30, "020");
    RETURN_VAL_IF_TRUE(percentage < 50, "040");
    RETURN_VAL_IF_TRUE(percentage < 70, "060");
    RETURN_VAL_IF_TRUE(percentage < 90, "080");
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