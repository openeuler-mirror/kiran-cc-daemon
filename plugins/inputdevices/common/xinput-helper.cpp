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
    KLOG_PROFILE("");

    int32_t n_devices = 0;
    auto devices_info = XListInputDevices(GDK_DISPLAY_XDISPLAY(gdk_display_get_default()), &n_devices);

    for (auto i = 0; i < n_devices; i++)
    {
        if (strcmp(devices_info[i].name, "Virtual core pointer") == 0 ||
            strcmp(devices_info[i].name, "Virtual core keyboard") == 0)
        {
            KLOG_DEBUG("ignore device: %s.", devices_info[i].name);
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