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

    // 获取图标名，按照device_types设置的设备类型顺序进行搜索
    std::string get_icon_name(const std::vector<uint32_t>& device_types);

    // 获取设备图标名
    std::string get_device_icon_name(std::shared_ptr<PowerUPowerDevice> upower_device);
    // 电量百分比转图标索引
    std::string percentage2index(int32_t percentage);

    void on_settings_changed(const Glib::ustring& key);
    void on_device_props_changed(std::shared_ptr<PowerUPowerDevice> upwer_device,
                                 const UPowerDeviceProps& old_props,
                                 const UPowerDeviceProps& new_props);

private:
    static PowerTray* instance_;

    PowerWrapperManager* wrapper_manager_;

    std::shared_ptr<PowerUPower> upower_client_;

    Glib::RefPtr<Gio::Settings> upower_settings_;

    GtkStatusIcon* status_icon_;
};
}  // namespace Kiran