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