/*
 * @Author       : tangjie02
 * @Date         : 2020-08-10 09:18:48
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-08-10 09:34:22
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/plugins/inputdevices/common/xinput-helper.cpp
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
}  // namespace Kiran