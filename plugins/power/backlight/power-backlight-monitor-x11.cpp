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

#include "plugins/power/backlight/power-backlight-monitor-x11.h"

#include <X11/Xatom.h>

namespace Kiran
{
PowerBacklightMonitorX11::PowerBacklightMonitorX11(Atom backlight_atom, RROutput output) : backlight_atom_(backlight_atom),
                                                                                           output_(output)
{
    this->display_ = gdk_display_get_default();
    this->xdisplay_ = GDK_DISPLAY_XDISPLAY(this->display_);
}

bool PowerBacklightMonitorX11::set_brightness_value(int32_t brightness_value)
{
    gdk_x11_display_error_trap_push(this->display_);

    XRRChangeOutputProperty(this->xdisplay_,
                            this->output_,
                            this->backlight_atom_,
                            XA_INTEGER,
                            32,
                            PropModeReplace,
                            (unsigned char *)&brightness_value,
                            1);
    gdk_display_flush(this->display_);
    if (gdk_x11_display_error_trap_pop(this->display_))
    {
        KLOG_WARNING("Failed to XRRChangeOutputProperty for brightness %i", brightness_value);
        return false;
    }
    return true;
}

int32_t PowerBacklightMonitorX11::get_brightness_value()
{
    RETURN_VAL_IF_TRUE(this->backlight_atom_ == None, -1);

    unsigned long nitems;
    unsigned long bytes_after;
    guint *prop;
    Atom actual_type;
    int actual_format;
    int32_t result = -1;

    if (XRRGetOutputProperty(this->xdisplay_,
                             this->output_,
                             this->backlight_atom_,
                             0,
                             4,
                             False,
                             False,
                             None,
                             &actual_type,
                             &actual_format,
                             &nitems,
                             &bytes_after,
                             ((unsigned char **)&prop)) != Success)
    {
        KLOG_WARNING("Failed to get brightness property for output %d.", (int32_t)this->output_);
        return -1;
    }

    if (actual_type == XA_INTEGER &&
        nitems == 1 &&
        actual_format == 32)
    {
        memcpy(&result, prop, sizeof(int32_t));
    }
    else
    {
        KLOG_WARNING("The data of the brightness proerty is incorrect.");
    }

    XFree(prop);

    return result;
}

bool PowerBacklightMonitorX11::get_brightness_range(int32_t &min, int32_t &max)
{
    XRRPropertyInfo *info = NULL;

    SCOPE_EXIT({
        if (info != NULL)
        {
            XFree(info);
        }
    });

    info = XRRQueryOutputProperty(this->xdisplay_, this->output_, this->backlight_atom_);
    if (info == NULL)
    {
        KLOG_WARNING("Could not get output property for %d.", (int32_t)this->output_);
        return false;
    }

    if (!info->range || info->num_values != 2)
    {
        KLOG_WARNING("The values isn't a range");
        return false;
    }
    min = info->values[0];
    max = info->values[1];
    return true;
}

}  // namespace Kiran
