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