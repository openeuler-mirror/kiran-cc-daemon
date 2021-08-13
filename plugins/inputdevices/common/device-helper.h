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

#include "lib/base/base.h"

// xlib.h must be defined after gdkmm giomm header file.
#include <X11/Xatom.h>
#include <X11/extensions/XInput.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>

#include <string>

namespace Kiran
{
class DeviceHelper
{
public:
    DeviceHelper() = delete;
    DeviceHelper(XDeviceInfo *device_info);
    virtual ~DeviceHelper();

    // 获取设备名
    std::string get_device_name();
    // 获取属性名对应的Atom，如果不存在不会新建且返回None
    Atom get_atom(const std::string &property_name);
    // 判断设备是否存在指定的属性名
    bool has_property(const std::string &property_name);
    // 判断设备是否为触摸板
    bool is_touchpad();
    // 设置属性值
    void set_property(const std::string &property_name, const std::vector<bool> &property_value);
    void set_property(const std::string &property_name, float property_value);

    operator bool() const
    {
        return (this->device_info_ != NULL && this->device_ != NULL);
    }

private:
    XDeviceInfo *device_info_;
    XDevice *device_;
};
}  // namespace Kiran