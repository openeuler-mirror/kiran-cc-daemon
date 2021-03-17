/**
 * @file          /kiran-cc-daemon/plugins/power/notification/power-notification-manager.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#pragma once

#include <libnotify/notification.h>

#include "plugins/power/wrapper/power-wrapper-manager.h"

namespace Kiran
{
class PowerNotificationManager
{
public:
    PowerNotificationManager(PowerWrapperManager* wrapper_manager);
    virtual ~PowerNotificationManager();

    static PowerNotificationManager* get_instance() { return instance_; };

    static void global_init(PowerWrapperManager* wrapper_manager);

    static void global_deinit() { delete instance_; };

private:
    void init();

    bool message_notify(const std::string& title,
                        const std::string& message,
                        uint32_t timeout,
                        const std::string& icon_name,
                        NotifyUrgency urgency);

    void on_device_status_changed(std::shared_ptr<PowerUPowerDevice> device, UPowerDeviceEvent event);
    void on_device_discharging(std::shared_ptr<PowerUPowerDevice> device);
    void on_device_fully_charged(std::shared_ptr<PowerUPowerDevice> device);
    void on_device_charge_low(std::shared_ptr<PowerUPowerDevice> device);
    void on_device_charge_critical(std::shared_ptr<PowerUPowerDevice> device);
    void on_device_charge_action(std::shared_ptr<PowerUPowerDevice> device);

private:
    static PowerNotificationManager* instance_;

    PowerWrapperManager* wrapper_manager_;

    std::shared_ptr<PowerUPower> upower_client_;

    NotifyNotification* device_notification_;

    Glib::RefPtr<Gio::Settings> power_settings_;
};
}  // namespace Kiran