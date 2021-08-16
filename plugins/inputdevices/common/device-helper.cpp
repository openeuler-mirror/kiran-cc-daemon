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

#include "plugins/inputdevices/common/device-helper.h"

namespace Kiran
{
DeviceHelper::DeviceHelper(XDeviceInfo *device_info) : device_info_(device_info),
                                                       device_(NULL)
{
    if (device_info_)
    {
        auto display = gdk_display_get_default();
        gdk_x11_display_error_trap_push(display);
        this->device_ = XOpenDevice(GDK_DISPLAY_XDISPLAY(display), device_info->id);
        if ((gdk_x11_display_error_trap_pop(display) != 0) || (this->device_ == NULL))
        {
            KLOG_WARNING("failed to open device: %s.", device_info->name);
        }
    }
}

DeviceHelper::~DeviceHelper()
{
    if (this->device_)
    {
        auto display = gdk_display_get_default();
        XCloseDevice(GDK_DISPLAY_XDISPLAY(display), this->device_);
    }
}
std::string DeviceHelper::get_device_name()
{
    return this->device_info_ ? this->device_info_->name : "NONE";
}

Atom DeviceHelper::get_atom(const std::string &property_name)
{
    return XInternAtom(GDK_DISPLAY_XDISPLAY(gdk_display_get_default()), property_name.c_str(), True);
}

bool DeviceHelper::has_property(const std::string &property_name)
{
    KLOG_PROFILE("device_name: %s property_name: %s.",
                 this->get_device_name().c_str(),
                 property_name.c_str());

    RETURN_VAL_IF_TRUE(this->device_ == NULL, false);

    Atom actual_type;
    int actual_format;
    unsigned long nitems, bytes_after;
    unsigned char *data;

    auto property = DeviceHelper::get_atom(property_name);
    RETURN_VAL_IF_TRUE(property == None, false);

    auto display = gdk_display_get_default();

    gdk_x11_display_error_trap_push(display);
    auto rc = XGetDeviceProperty(GDK_DISPLAY_XDISPLAY(display),
                                 this->device_,
                                 property,
                                 0,
                                 1,
                                 False,
                                 AnyPropertyType,
                                 &actual_type,
                                 &actual_format,
                                 &nitems,
                                 &bytes_after,
                                 &data);
    gdk_x11_display_error_trap_pop_ignored(display);

    if (rc == Success)
    {
        XFree(data);
    }

    return (rc == Success && actual_type != None);
}

bool DeviceHelper::is_touchpad()
{
    KLOG_PROFILE("device_name: %s.", this->get_device_name().c_str());

    RETURN_VAL_IF_TRUE(this->device_ == NULL, false);

    auto display = gdk_display_get_default();

    if (this->device_info_->type != XInternAtom(GDK_DISPLAY_XDISPLAY(display), XI_TOUCHPAD, True))
    {
        return false;
    }

    if (this->has_property("libinput Tapping Enabled") || this->has_property("Synaptics Off"))
    {
        return true;
    }

    return false;
}

void DeviceHelper::set_property(const std::string &property_name, const std::vector<bool> &property_value)
{
    KLOG_PROFILE("device_name: %s property_name: %s property_value: %s.",
                 this->device_info_->name,
                 property_name.c_str(),
                 StrUtils::join(property_value, ",").c_str());

    RETURN_IF_TRUE(this->device_ == NULL);

    unsigned long nitems, bytes_after;
    unsigned char *data;
    int actual_format;
    Atom actual_type;

    auto property = this->get_atom(property_name);
    RETURN_IF_TRUE(property == None);

    auto display = gdk_display_get_default();
    gdk_x11_display_error_trap_push(display);
    auto rc = XGetDeviceProperty(GDK_DISPLAY_XDISPLAY(display),
                                 this->device_,
                                 property,
                                 0,
                                 1,
                                 False,
                                 XA_INTEGER,
                                 &actual_type,
                                 &actual_format,
                                 &nitems,
                                 &bytes_after,
                                 &data);

    if (rc == Success && actual_type == XA_INTEGER && actual_format == 8 && nitems > 0)
    {
        if (property_value.size() > nitems)
        {
            KLOG_WARNING("ignore the remaining %d value. the number of property set: %d, the number of real device property: %d.",
                         property_value.size() - nitems,
                         property_value.size(),
                         nitems);
        }
        uint32_t num = std::min(uint32_t(nitems), uint32_t(property_value.size()));
        for (uint32_t i = 0; i < num; ++i)
        {
            data[i] = property_value[i] ? 1 : 0;
        }
        XChangeDeviceProperty(GDK_DISPLAY_XDISPLAY(display),
                              this->device_,
                              property,
                              XA_INTEGER,
                              8,
                              PropModeReplace,
                              data,
                              nitems);
    }

    if (rc == Success)
    {
        XFree(data);
    }

    if (gdk_x11_display_error_trap_pop(display))
    {
        KLOG_WARNING("failed to set property '%s' for device '%s'.", property_name.c_str(), this->device_info_->name);
    }
}

void DeviceHelper::set_property(const std::string &property_name, float property_value)
{
    KLOG_PROFILE("property_name: %s property_value: %f.", property_name.c_str(), property_value);
    RETURN_IF_TRUE(this->device_ == NULL);

    unsigned long nitems, bytes_after;
    unsigned char *data;
    int actual_format;
    Atom actual_type;

    auto property = this->get_atom(property_name);
    RETURN_IF_TRUE(property == None);

    auto float_type = this->get_atom("FLOAT");
    RETURN_IF_TRUE(float_type == None);

    auto display = gdk_display_get_default();
    gdk_x11_display_error_trap_push(display);
    auto rc = XGetDeviceProperty(GDK_DISPLAY_XDISPLAY(display),
                                 this->device_,
                                 property,
                                 0,
                                 1,
                                 False,
                                 float_type,
                                 &actual_type,
                                 &actual_format,
                                 &nitems,
                                 &bytes_after,
                                 &data);

    if (rc == Success && actual_type == float_type && actual_format == 32 && nitems > 0)
    {
        *(float *)data = property_value;
        XChangeDeviceProperty(GDK_DISPLAY_XDISPLAY(display),
                              this->device_,
                              property,
                              float_type,
                              32,
                              PropModeReplace,
                              data,
                              nitems);
    }

    if (rc == Success)
    {
        XFree(data);
    }

    if (gdk_x11_display_error_trap_pop(display))
    {
        KLOG_WARNING("failed to set property '%s' for device '%s'.", property_name.c_str(), this->device_info_->name);
    }
}

}  // namespace Kiran
