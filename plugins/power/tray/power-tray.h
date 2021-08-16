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