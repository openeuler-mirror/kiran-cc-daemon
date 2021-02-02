/**
 * @file          /kiran-cc-daemon/plugins/power/backlight/power-backlight-monitor-x11.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
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
        LOG_WARNING("Failed to XRRChangeOutputProperty for brightness %i", brightness_value);
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
        LOG_WARNING("Failed to get brightness property for output %d.", (int32_t)this->output_);
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
        LOG_WARNING("The data of the brightness proerty is incorrect.");
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
        LOG_WARNING("Could not get output property for %d.", (int32_t)this->output_);
        return false;
    }

    if (!info->range || info->num_values != 2)
    {
        LOG_WARNING("The values isn't a range");
        return false;
    }
    min = info->values[0];
    max = info->values[1];
    return true;
}

}  // namespace Kiran
