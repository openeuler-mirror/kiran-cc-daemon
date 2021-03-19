/**
 * @file          /kiran-cc-daemon/plugins/power/tray/power-tray.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#include "plugins/power/tray/power-tray.h"

#include "power_i.h"

namespace Kiran
{
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

    auto display_device = this->upower_client_->get_display_device();
    display_device->signal_props_changed().connect(sigc::mem_fun(this, &PowerTray::on_display_device_props_changed));
}

void PowerTray::update_status_icon()
{
    auto display_device = this->upower_client_->get_display_device();
    const auto& props = display_device->get_props();
    auto visible = this->upower_settings_->get_boolean(POWER_SCHEMA_SHOW_TRAY_ICON);

    LOG_DEBUG("icon name: %s, is present: %d.", props.icon_name.c_str(), props.is_present);

    if (props.icon_name.empty() || !props.is_present || !visible)
    {
        gtk_status_icon_set_visible(this->status_icon_, false);
    }
    else
    {
        gtk_status_icon_set_from_icon_name(this->status_icon_, props.icon_name.c_str());
        gtk_status_icon_set_visible(this->status_icon_, true);
    }
}

void PowerTray::on_settings_changed(const Glib::ustring& key)
{
    switch (shash(key.c_str()))
    {
    case CONNECT(POWER_SCHEMA_SHOW_TRAY_ICON, _hash):
        this->update_status_icon();
        break;
    default:
        break;
    }
}

void PowerTray::on_display_device_props_changed(const UPowerDeviceProps& old_props,
                                                const UPowerDeviceProps& new_props)
{
    if (old_props.icon_name != new_props.icon_name)
    {
        this->update_status_icon();
    }
}
}  // namespace Kiran