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

#include "plugins/power/power-manager.h"

#include "plugins/power/backlight/power-backlight.h"
#include "plugins/power/power-utils.h"
#include "power-i.h"

namespace Kiran
{
PowerManager::PowerManager(PowerWrapperManager* wrapper_manager, PowerBacklight* backlight) : wrapper_manager_(wrapper_manager),
                                                                                              backlight_(backlight),
                                                                                              dbus_connect_id_(),
                                                                                              object_register_id_(0)
{
    this->power_settings_ = Gio::Settings::create(POWER_SCHEMA_ID);
    this->upower_client_ = this->wrapper_manager_->get_default_upower();
    this->profiles_ = this->wrapper_manager_->get_default_profiles();
}

PowerManager::~PowerManager()
{
    if (this->dbus_connect_id_)
    {
        Gio::DBus::unown_name(this->dbus_connect_id_);
    }
}

PowerManager* PowerManager::instance_ = nullptr;
void PowerManager::global_init(PowerWrapperManager* wrapper_manager, PowerBacklight* backlight)
{
    instance_ = new PowerManager(wrapper_manager, backlight);
    instance_->init();
}

void PowerManager::init()
{
    this->upower_client_->signal_on_battery_changed().connect(sigc::mem_fun(this, &PowerManager::on_battery_changed));
    this->upower_client_->signal_lid_is_closed_changed().connect(sigc::mem_fun(this, &PowerManager::on_lid_is_closed_changed));
    this->power_settings_->signal_changed().connect(sigc::mem_fun(this, &PowerManager::on_settings_changed));
    this->backlight_->signal_brightness_changed().connect(sigc::mem_fun(this, &PowerManager::on_brightness_changed));

    this->dbus_connect_id_ = Gio::DBus::own_name(Gio::DBus::BUS_TYPE_SESSION,
                                                 POWER_DBUS_NAME,
                                                 sigc::mem_fun(this, &PowerManager::on_bus_acquired),
                                                 sigc::mem_fun(this, &PowerManager::on_name_acquired),
                                                 sigc::mem_fun(this, &PowerManager::on_name_lost));

    this->profiles_->signal_active_profile_changed().connect(sigc::mem_fun(this, &PowerManager::on_active_profile_changed));
}

void PowerManager::SetIdleAction(gint32 device,
                                 gint32 supply,
                                 gint32 idle_timeout,
                                 gint32 action,
                                 MethodInvocation& invocation)
{
    KLOG_DEBUG_POWER("Set idle action for device %s which supply=%s, idle timeout=%d, action=%s.",
                     PowerUtils::device_enum2str(device).c_str(),
                     PowerUtils::supply_enum2str(supply).c_str(),
                     idle_timeout,
                     PowerUtils::action_enum2str(action).c_str());

    if (action < 0 || action >= PowerAction::POWER_ACTION_LAST)
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_POWER_UNKNOWN_ACTION_1);
    }

    switch (device)
    {
    case PowerDeviceType::POWER_DEVICE_TYPE_COMPUTER:
    {
        switch (supply)
        {
        case PowerSupplyMode::POWER_SUPPLY_MODE_BATTERY:
            this->power_settings_->set_int(POWER_SCHEMA_COMPUTER_BATTERY_IDLE_TIME, idle_timeout);
            this->power_settings_->set_enum(POWER_SCHEMA_COMPUTER_BATTERY_IDLE_ACTION, action);
            break;
        case PowerSupplyMode::POWER_SUPPLY_MODE_AC:
            this->power_settings_->set_int(POWER_SCHEMA_COMPUTER_AC_IDLE_TIME, idle_timeout);
            this->power_settings_->set_enum(POWER_SCHEMA_COMPUTER_AC_IDLE_ACTION, action);
            break;
        default:
            DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_POWER_SUPPLY_MODE_UNSUPPORTED_1);
        }
        break;
    }
    case PowerDeviceType::POWER_DEVICE_TYPE_BACKLIGHT:
    {
        switch (supply)
        {
        case PowerSupplyMode::POWER_SUPPLY_MODE_BATTERY:
            this->power_settings_->set_int(POWER_SCHEMA_BACKLIGHT_BATTERY_IDLE_TIME, idle_timeout);
            this->power_settings_->set_enum(POWER_SCHEMA_BACKLIGHT_BATTERY_IDLE_ACTION, action);
            break;
        case PowerSupplyMode::POWER_SUPPLY_MODE_AC:
            this->power_settings_->set_int(POWER_SCHEMA_BACKLIGHT_AC_IDLE_TIME, idle_timeout);
            this->power_settings_->set_enum(POWER_SCHEMA_BACKLIGHT_AC_IDLE_ACTION, action);
            break;
        default:
            DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_POWER_SUPPLY_MODE_UNSUPPORTED_2);
        }
        break;
    }
    default:
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_POWER_DEVICE_UNSUPPORTED_1);
        break;
    }
    invocation.ret();
}

void PowerManager::GetIdleAction(gint32 device,
                                 gint32 supply,
                                 MethodInvocation& invocation)
{
    int32_t idle_timeout = 0;
    int32_t action = PowerAction::POWER_ACTION_NOTHING;

    switch (device)
    {
    case PowerDeviceType::POWER_DEVICE_TYPE_COMPUTER:
    {
        switch (supply)
        {
        case PowerSupplyMode::POWER_SUPPLY_MODE_BATTERY:
            idle_timeout = this->power_settings_->get_int(POWER_SCHEMA_COMPUTER_BATTERY_IDLE_TIME);
            action = this->power_settings_->get_enum(POWER_SCHEMA_COMPUTER_BATTERY_IDLE_ACTION);
            break;
        case PowerSupplyMode::POWER_SUPPLY_MODE_AC:
            idle_timeout = this->power_settings_->get_int(POWER_SCHEMA_COMPUTER_AC_IDLE_TIME);
            action = this->power_settings_->get_enum(POWER_SCHEMA_COMPUTER_AC_IDLE_ACTION);
            break;
        default:
            DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_POWER_SUPPLY_MODE_UNSUPPORTED_3);
        }
        break;
    }
    case PowerDeviceType::POWER_DEVICE_TYPE_BACKLIGHT:
    {
        switch (supply)
        {
        case PowerSupplyMode::POWER_SUPPLY_MODE_BATTERY:
            idle_timeout = this->power_settings_->get_int(POWER_SCHEMA_BACKLIGHT_BATTERY_IDLE_TIME);
            action = this->power_settings_->get_enum(POWER_SCHEMA_BACKLIGHT_BATTERY_IDLE_ACTION);
            break;
        case PowerSupplyMode::POWER_SUPPLY_MODE_AC:
            idle_timeout = this->power_settings_->get_int(POWER_SCHEMA_BACKLIGHT_AC_IDLE_TIME);
            action = this->power_settings_->get_enum(POWER_SCHEMA_BACKLIGHT_AC_IDLE_ACTION);
            break;
        default:
            DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_POWER_SUPPLY_MODE_UNSUPPORTED_4);
        }
        break;
    }
    default:
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_POWER_DEVICE_UNSUPPORTED_2);
        break;
    }

    invocation.ret(std::make_tuple(idle_timeout, action));
}

void PowerManager::SetEventAction(gint32 event,
                                  gint32 action,
                                  MethodInvocation& invocation)
{
    if (action < 0 || action >= PowerAction::POWER_ACTION_LAST)
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_POWER_UNKNOWN_ACTION_2);
    }
    bool result = false;

    switch (event)
    {
    case PowerEvent::POWER_EVENT_RELEASE_POWEROFF:
        result = this->power_settings_->set_enum(POWER_SCHEMA_BUTTON_POWER_ACTION, action);
        break;
    case PowerEvent::POWER_EVENT_PRESSED_SLEEP:
    case PowerEvent::POWER_EVENT_PRESSED_SUSPEND:
        result = this->power_settings_->set_enum(POWER_SCHEMA_BUTTON_SUSPEND_ACTION, action);
        break;
    case PowerEvent::POWER_EVENT_PRESSED_HIBERNATE:
        result = this->power_settings_->set_enum(POWER_SCHEMA_BUTTON_HIBERNATE_ACTION, action);
        break;
    case PowerEvent::POWER_EVENT_LID_CLOSED:
        result = this->power_settings_->set_enum(POWER_SCHEMA_LID_CLOSED_ACTION, action);
        break;
    case PowerEvent::POWER_EVENT_BATTERY_CHARGE_ACTION:
        result = this->power_settings_->set_enum(POWER_SCHEMA_BATTERY_CRITICAL_ACTION, action);
        break;
    default:
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_POWER_EVENT_UNSUPPORTED_1);
    }

    if (!result)
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_POWER_SET_ACTION_FAILED);
    }
    else
    {
        invocation.ret();
    }
}

void PowerManager::GetEventAction(gint32 event,
                                  MethodInvocation& invocation)
{
    int32_t action = PowerAction::POWER_ACTION_NOTHING;

    switch (event)
    {
    case PowerEvent::POWER_EVENT_RELEASE_POWEROFF:
        action = this->power_settings_->get_enum(POWER_SCHEMA_BUTTON_POWER_ACTION);
        break;
    case PowerEvent::POWER_EVENT_PRESSED_SLEEP:
    case PowerEvent::POWER_EVENT_PRESSED_SUSPEND:
        action = this->power_settings_->get_enum(POWER_SCHEMA_BUTTON_SUSPEND_ACTION);
        break;
    case PowerEvent::POWER_EVENT_PRESSED_HIBERNATE:
        action = this->power_settings_->get_enum(POWER_SCHEMA_BUTTON_HIBERNATE_ACTION);
        break;
    case PowerEvent::POWER_EVENT_LID_CLOSED:
        action = this->power_settings_->get_enum(POWER_SCHEMA_LID_CLOSED_ACTION);
        break;
    case PowerEvent::POWER_EVENT_BATTERY_CHARGE_ACTION:
        action = this->power_settings_->get_enum(POWER_SCHEMA_BATTERY_CRITICAL_ACTION);
        break;
    default:
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_POWER_EVENT_UNSUPPORTED_2);
    }

    invocation.ret(action);
}

void PowerManager::SetBrightness(gint32 device,
                                 gint32 brightness_percentage,
                                 MethodInvocation& invocation)
{
    KLOG_DEBUG_POWER("Set brightness percentage of device %s to %d.",
                     PowerUtils::device_enum2str(device).c_str(),
                     brightness_percentage);

    bool result = false;

    switch (device)
    {
    case PowerDeviceType::POWER_DEVICE_TYPE_MONITOR:
    {
        auto monitor = PowerBacklight::get_instance()->get_backlight_device(PowerDeviceType::POWER_DEVICE_TYPE_MONITOR);
        result = monitor->set_brightness(brightness_percentage);
        break;
    }
    case PowerDeviceType::POWER_DEVICE_TYPE_KBD:
    {
        auto kbd = PowerBacklight::get_instance()->get_backlight_device(PowerDeviceType::POWER_DEVICE_TYPE_KBD);
        result = kbd->set_brightness(brightness_percentage);
        break;
    }
    default:
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_POWER_DEVICE_UNSUPPORTED_3);
    }

    if (!result)
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_POWER_SET_BRIGHTNESS_FAILED);
    }
    else
    {
        invocation.ret();
    }
}

void PowerManager::GetBrightness(gint32 device,
                                 MethodInvocation& invocation)
{
    KLOG_DEBUG_POWER("Get brightness percentage of device %s.", PowerUtils::device_enum2str(device).c_str());
    int32_t brightness_percentage = -1;

    switch (device)
    {
    case PowerDeviceType::POWER_DEVICE_TYPE_MONITOR:
    {
        auto monitor = PowerBacklight::get_instance()->get_backlight_device(PowerDeviceType::POWER_DEVICE_TYPE_MONITOR);
        brightness_percentage = monitor->get_brightness();
        break;
    }
    case PowerDeviceType::POWER_DEVICE_TYPE_KBD:
    {
        auto kbd = PowerBacklight::get_instance()->get_backlight_device(PowerDeviceType::POWER_DEVICE_TYPE_KBD);
        brightness_percentage = kbd->get_brightness();
        break;
    }
    default:
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_POWER_DEVICE_UNSUPPORTED_4);
    }

    invocation.ret(brightness_percentage);
}

void PowerManager::EnableDisplayIdleDimmed(bool enabled, MethodInvocation& invocation)
{
    this->DisplayIdleDimmedEnabled_set(enabled);
    invocation.ret();
}

void PowerManager::EnableChargeLowDimmed(bool enabled, MethodInvocation& invocation)
{
    this->ChargeLowDimmedEnabled_set(enabled);
    invocation.ret();
}

void PowerManager::EnableChargeLowSaver(bool enabled, MethodInvocation& invocation)
{
    this->ChargeLowSaverEnabled_set(enabled);
    invocation.ret();
}

void PowerManager::SwitchProfile(gint32 mode, MethodInvocation& invocation)
{
    this->ActiveProfile_set(mode);
    invocation.ret();
}

bool PowerManager::DisplayIdleDimmedEnabled_setHandler(bool value)
{
    this->power_settings_->set_boolean(POWER_SCHEMA_ENABLE_DISPLAY_IDLE_DIMMED, value);
    return true;
}

bool PowerManager::ChargeLowDimmedEnabled_setHandler(bool value)
{
    this->power_settings_->set_boolean(POWER_SCHEMA_ENABLE_CHARGE_LOW_DIMMED, value);
    return true;
}

bool PowerManager::ChargeLowSaverEnabled_setHandler(bool value)
{
    this->power_settings_->set_boolean(POWER_SCHEMA_ENABLE_CHARGE_LOW_SAVER, value);
    return true;
}

bool PowerManager::ActiveProfile_setHandler(gint32 value)
{
    return this->profiles_->switch_profile(value);
}

bool PowerManager::OnBattery_get()
{
    return this->upower_client_->get_on_battery();
}

bool PowerManager::LidIsPresent_get()
{
    return this->upower_client_->get_lid_is_present();
}

bool PowerManager::DisplayIdleDimmedEnabled_get()
{
    return this->power_settings_->get_boolean(POWER_SCHEMA_ENABLE_DISPLAY_IDLE_DIMMED);
}

bool PowerManager::ChargeLowDimmedEnabled_get()
{
    return this->power_settings_->get_boolean(POWER_SCHEMA_ENABLE_CHARGE_LOW_DIMMED);
}

bool PowerManager::ChargeLowSaverEnabled_get()
{
    return this->power_settings_->get_boolean(POWER_SCHEMA_ENABLE_CHARGE_LOW_SAVER);
}

gint32 PowerManager::ActiveProfile_get()
{
    return this->profiles_->get_active_profile();
}

void PowerManager::on_battery_changed(bool on_battery)
{
    this->OnBattery_set(on_battery);
}

void PowerManager::on_lid_is_closed_changed(bool lid_is_closed)
{
    this->LidIsPresent_set(lid_is_closed);
}

void PowerManager::on_settings_changed(const Glib::ustring& key)
{
    switch (shash(key.c_str()))
    {
    case CONNECT(POWER_SCHEMA_COMPUTER_BATTERY_IDLE_TIME, _hash):
    case CONNECT(POWER_SCHEMA_COMPUTER_BATTERY_IDLE_ACTION, _hash):
        this->IdleActionChanged_signal.emit(PowerDeviceType::POWER_DEVICE_TYPE_COMPUTER, PowerSupplyMode::POWER_SUPPLY_MODE_BATTERY);
        break;
    case CONNECT(POWER_SCHEMA_COMPUTER_AC_IDLE_TIME, _hash):
    case CONNECT(POWER_SCHEMA_COMPUTER_AC_IDLE_ACTION, _hash):
        this->IdleActionChanged_signal.emit(PowerDeviceType::POWER_DEVICE_TYPE_COMPUTER, PowerSupplyMode::POWER_SUPPLY_MODE_AC);
        break;
    case CONNECT(POWER_SCHEMA_BACKLIGHT_BATTERY_IDLE_TIME, _hash):
    case CONNECT(POWER_SCHEMA_BACKLIGHT_BATTERY_IDLE_ACTION, _hash):
        this->IdleActionChanged_signal.emit(PowerDeviceType::POWER_DEVICE_TYPE_BACKLIGHT, PowerSupplyMode::POWER_SUPPLY_MODE_BATTERY);
        break;
    case CONNECT(POWER_SCHEMA_BACKLIGHT_AC_IDLE_TIME, _hash):
    case CONNECT(POWER_SCHEMA_BACKLIGHT_AC_IDLE_ACTION, _hash):
        this->IdleActionChanged_signal.emit(PowerDeviceType::POWER_DEVICE_TYPE_BACKLIGHT, PowerSupplyMode::POWER_SUPPLY_MODE_AC);
        break;
    case CONNECT(POWER_SCHEMA_BUTTON_SUSPEND_ACTION, _hash):
        this->EventActionChanged_signal.emit(PowerEvent::POWER_EVENT_PRESSED_SUSPEND);
        break;
    case CONNECT(POWER_SCHEMA_BUTTON_HIBERNATE_ACTION, _hash):
        this->EventActionChanged_signal.emit(PowerEvent::POWER_EVENT_PRESSED_HIBERNATE);
        break;
    case CONNECT(POWER_SCHEMA_BUTTON_POWER_ACTION, _hash):
        this->EventActionChanged_signal.emit(PowerEvent::POWER_EVENT_RELEASE_POWEROFF);
        break;
    case CONNECT(POWER_SCHEMA_LID_CLOSED_ACTION, _hash):
        this->EventActionChanged_signal.emit(PowerEvent::POWER_EVENT_LID_CLOSED);
        break;
    default:
        break;
    }
}

void PowerManager::on_brightness_changed(std::shared_ptr<PowerBacklightPercentage> backlight_device, int32_t brightness_value)
{
    KLOG_DEBUG_POWER("Changed brightness to %d, type is %d.", brightness_value, backlight_device->get_type());

    this->BrightnessChanged_signal.emit(backlight_device->get_type());
}

void PowerManager::on_active_profile_changed(int32_t profile_mode)
{
    this->ActiveProfileChanged_signal.emit(profile_mode);
}

void PowerManager::on_bus_acquired(const Glib::RefPtr<Gio::DBus::Connection>& connect, Glib::ustring name)
{
    if (!connect)
    {
        KLOG_WARNING_POWER("Failed to connect dbus with %s", name.c_str());
        return;
    }
    try
    {
        this->object_register_id_ = this->register_object(connect, POWER_OBJECT_PATH);
    }
    catch (const Glib::Error& e)
    {
        KLOG_WARNING_POWER("Register object_path %s fail: %s.", POWER_OBJECT_PATH, e.what().c_str());
    }
}

void PowerManager::on_name_acquired(const Glib::RefPtr<Gio::DBus::Connection>& connect, Glib::ustring name)
{
    KLOG_DEBUG_POWER("Success to register dbus name: %s", name.c_str());
}

void PowerManager::on_name_lost(const Glib::RefPtr<Gio::DBus::Connection>& connect, Glib::ustring name)
{
    KLOG_WARNING_POWER("Failed to register dbus name: %s", name.c_str());
}

}  // namespace Kiran