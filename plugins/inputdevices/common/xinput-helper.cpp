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

#include "plugins/inputdevices/common/xinput-helper.h"

#include <gdk/gdk.h>
#include <gdk/gdkx.h>

namespace Kiran
{
bool XInputHelper::supports_xinput_devices()
{
    int op_code, event, error;

    return XQueryExtension(GDK_DISPLAY_XDISPLAY(gdk_display_get_default()),
                           "XInputExtension",
                           &op_code,
                           &event,
                           &error);
}

void XInputHelper::foreach_device(std::function<void(std::shared_ptr<DeviceHelper>)> callback)
{
    int32_t n_devices = 0;
    auto devices_info = XListInputDevices(GDK_DISPLAY_XDISPLAY(gdk_display_get_default()), &n_devices);

    for (auto i = 0; i < n_devices; i++)
    {
        if (strcmp(devices_info[i].name, "Virtual core pointer") == 0 ||
            strcmp(devices_info[i].name, "Virtual core keyboard") == 0)
        {
            KLOG_DEBUG_INPUTDEVICES("Ignore device: %s.", devices_info[i].name);
            continue;
        }
        auto device_helper = std::make_shared<DeviceHelper>(&devices_info[i]);
        if (device_helper)
        {
            callback(device_helper);
        }
    }

    if (devices_info != NULL)
    {
        XFreeDeviceList(devices_info);
    }
}
}  // namespace Kiran