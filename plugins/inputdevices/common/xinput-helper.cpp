/**
 * @file          /kiran-cc-daemon/plugins/inputdevices/common/xinput-helper.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
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
    SETTINGS_PROFILE("");

    int32_t n_devices = 0;
    auto devices_info = XListInputDevices(GDK_DISPLAY_XDISPLAY(gdk_display_get_default()), &n_devices);

    for (auto i = 0; i < n_devices; i++)
    {
        if (strcmp(devices_info[i].name, "Virtual core pointer") == 0 ||
            strcmp(devices_info[i].name, "Virtual core keyboard") == 0)
        {
            LOG_DEBUG("ignore device: %s.", devices_info[i].name);
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