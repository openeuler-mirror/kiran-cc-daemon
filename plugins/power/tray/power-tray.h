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