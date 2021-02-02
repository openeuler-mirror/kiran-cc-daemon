/**
 * @file          /kiran-cc-daemon/plugins/power/tray/power-tray.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#pragma once

#include <gtk/gtk.h>

#include "plugins/power/wrapper/power-wrapper-manager.h"

namespace Kiran
{
class PowerTray
{
public:
    PowerTray(PowerWrapperManager* wrapper_manager);
    virtual ~PowerTray();

    static PowerTray* get_instance() { return instance_; };

    static void global_init(PowerWrapperManager* wrapper_manager);

    static void global_deinit() { delete instance_; };

private:
    void init();

    // 更新托盘图标
    void update_status_icon();

    void on_settings_changed(const Glib::ustring& key);
    void on_display_device_props_changed(const UPowerDeviceProps& old_props, const UPowerDeviceProps& new_props);

private:
    static PowerTray* instance_;

    PowerWrapperManager* wrapper_manager_;

    std::shared_ptr<PowerUPower> upower_client_;

    Glib::RefPtr<Gio::Settings> upower_settings_;

    GtkStatusIcon* status_icon_;
};
}  // namespace Kiran