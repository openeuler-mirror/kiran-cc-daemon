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

#include "plugins/inputdevices/common/device-helper.h"

namespace Kiran
{
class XInputHelper
{
public:
    XInputHelper(){};
    virtual ~XInputHelper(){};

    // 判断是否支持xinput扩展
    static bool supports_xinput_devices();
    // 遍历输入设备
    static void foreach_device(std::function<void(std::shared_ptr<DeviceHelper>)> callback);

    // 监听输入设备热插拔（XI2 HierarchyChanged），可重复调用
    static bool start_device_monitor();
    static void stop_device_monitor();
    static sigc::signal<void> &signal_device_changed();

private:
    static GdkFilterReturn on_xevent(GdkXEvent *gdk_event, GdkEvent *event, gpointer data);
};
}  // namespace Kiran
