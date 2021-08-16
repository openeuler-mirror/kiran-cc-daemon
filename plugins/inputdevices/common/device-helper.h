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