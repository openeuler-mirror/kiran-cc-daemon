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

#include <X11/extensions/XInput2.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>

#include <algorithm>
#include <cstring>

namespace Kiran
{
namespace
{
struct DeviceMonitorState
{
    bool started = false;
    int xi_opcode = 0;
    sigc::signal<void> device_changed;
    sigc::connection idle_emit;
};

DeviceMonitorState &device_monitor_state()
{
    static DeviceMonitorState state;
    return state;
}

bool emit_device_changed_idle()
{
    auto &state = device_monitor_state();
    KLOG_DEBUG_INPUTDEVICES("Emitting device changed signal.");
    state.device_changed.emit();
    return false;
}

void schedule_device_changed_emit()
{
    auto &state = device_monitor_state();
    // 合并短时间内的多次热插拔事件，并避免在 X 事件回调里同步操作 X11。
    if (!state.idle_emit)
    {
        state.idle_emit = Glib::signal_idle().connect(sigc::ptr_fun(&emit_device_changed_idle));
    }
}

// 在保留本 client 已有 XIAllDevices 掩码的前提下，追加 XI_HierarchyChanged，
// 避免 XISelectEvents 覆盖 GDK 在同一 Display/root 上已选择的事件。
bool select_hierarchy_changed_events(Display *dpy, Window win)
{
    unsigned char mask[XIMaskLen(XI_LASTEVENT)] = {0};
    int mask_len = XIMaskLen(XI_HierarchyChanged);

    int nmasks = 0;
    XIEventMask *selected = XIGetSelectedEvents(dpy, win, &nmasks);
    if (nmasks < 0)
    {
        KLOG_WARNING_INPUTDEVICES("Failed to get selected XI events.");
        return false;
    }

    if (selected != nullptr)
    {
        for (int i = 0; i < nmasks; ++i)
        {
            if (selected[i].deviceid != XIAllDevices)
            {
                continue;
            }

            const int copy_len = std::min(selected[i].mask_len, static_cast<int>(sizeof(mask)));
            memcpy(mask, selected[i].mask, copy_len);
            mask_len = std::max(copy_len, XIMaskLen(XI_HierarchyChanged));
            break;
        }
        XFree(selected);
    }

    XISetMask(mask, XI_HierarchyChanged);

    XIEventMask evmask;
    evmask.deviceid = XIAllDevices;
    evmask.mask_len = mask_len;
    evmask.mask = mask;
    XISelectEvents(dpy, win, &evmask, 1);
    return true;
}
}  // namespace

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

bool XInputHelper::start_device_monitor()
{
    auto &state = device_monitor_state();
    RETURN_VAL_IF_TRUE(state.started, true);
    RETURN_VAL_IF_FALSE(supports_xinput_devices(), false);

    auto display = gdk_display_get_default();
    Display *dpy = GDK_DISPLAY_XDISPLAY(display);
    auto *root_window = gdk_get_default_root_window();
    RETURN_VAL_IF_FALSE(root_window != nullptr, false);

    int event = 0;
    int error = 0;
    if (!XQueryExtension(dpy, "XInputExtension", &state.xi_opcode, &event, &error))
    {
        KLOG_WARNING_INPUTDEVICES("Failed to query XInput extension.");
        return false;
    }

    if (!select_hierarchy_changed_events(dpy, gdk_x11_window_get_xid(root_window)))
    {
        KLOG_WARNING_INPUTDEVICES("Failed to select XI hierarchy events.");
        return false;
    }
    XFlush(dpy);

    gdk_window_add_filter(NULL, &XInputHelper::on_xevent, nullptr);
    state.started = true;

    KLOG_DEBUG_INPUTDEVICES("Registered XInput hierarchy event listener.");
    return true;
}

void XInputHelper::stop_device_monitor()
{
    auto &state = device_monitor_state();
    RETURN_IF_FALSE(state.started);

    state.idle_emit.disconnect();
    gdk_window_remove_filter(NULL, &XInputHelper::on_xevent, nullptr);
    state.started = false;
}

sigc::signal<void> &XInputHelper::signal_device_changed()
{
    return device_monitor_state().device_changed;
}

GdkFilterReturn XInputHelper::on_xevent(GdkXEvent *gdk_event, GdkEvent *event, gpointer data)
{
    (void)event;
    (void)data;

    XEvent *xev = static_cast<XEvent *>(gdk_event);
    auto &state = device_monitor_state();

    if (xev->type != GenericEvent || xev->xcookie.extension != state.xi_opcode)
    {
        return GDK_FILTER_CONTINUE;
    }

    // GDK 已调用过 XGetEventData，filter 内禁止再调用 XFreeEventData。
    if (xev->xcookie.evtype != XI_HierarchyChanged || xev->xcookie.data == nullptr)
    {
        return GDK_FILTER_CONTINUE;
    }

    auto *hierarchy_event = static_cast<XIHierarchyEvent *>(xev->xcookie.data);
    bool device_changed = false;

    for (int i = 0; i < hierarchy_event->num_info; ++i)
    {
        const auto flags = hierarchy_event->info[i].flags;
        if (flags & (XISlaveAdded | XISlaveRemoved | XIDeviceEnabled | XIDeviceDisabled))
        {
            device_changed = true;
            KLOG_DEBUG_INPUTDEVICES("Device hierarchy changed, device id: %d, flags: 0x%x.",
                                    hierarchy_event->info[i].deviceid,
                                    flags);
        }
    }

    if (device_changed)
    {
        schedule_device_changed_emit();
    }

    return GDK_FILTER_CONTINUE;
}
}  // namespace Kiran
