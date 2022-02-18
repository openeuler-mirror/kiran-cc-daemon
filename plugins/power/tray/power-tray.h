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

#include <gtkmm.h>

#include "plugins/power/wrapper/power-wrapper-manager.h"

namespace Kiran
{
class PowerTray
{
public:
    PowerTray();
    virtual ~PowerTray();

    static PowerTray* get_instance() { return instance_; };

    static void global_init();

    static void global_deinit() { delete instance_; };

    // 更新托盘图标
    void update_status_icon();

private:
    void init();

    void delay_update_status_icon();

    // 获取托盘图标对应的设备和需要显示的图标名
    std::shared_ptr<PowerUPowerDevice> get_device_for_tray(const std::vector<uint32_t>& device_types, std::string& icon_name);
    Glib::RefPtr<Gdk::Pixbuf> get_pixbuf_by_icon_name(const std::string& icon_name);

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
    Glib::RefPtr<Gdk::Pixbuf> icon_pixbuf_;
    sigc::connection update_icon_handler_;
};
}  // namespace Kiran