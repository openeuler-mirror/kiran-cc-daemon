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
 * Author:     meizhigang <meizhigang@kylinos.com.cn>
 */

#include <stdlib.h>

#include "clipboard-i.h"
#include "plugins/clipboard/clipboard-utils.h"

namespace Kiran
{
Atom XA_NULL;
Atom XA_TARGETS;
Atom XA_TIMESTAMP;
Atom XA_SAVE_TARGETS;
Atom XA_ATOM_PAIR;
Atom XA_DELETE;
Atom XA_INCR;
Atom XA_MULTIPLE;
Atom XA_INSERT_PROPERTY;
Atom XA_INSERT_SELECTION;
Atom XA_MANAGER;
Atom XA_CLIPBOARD_MANAGER;
Atom XA_CLIPBOARD;

unsigned long SELECTION_MAX_SIZE = 0;

void ClipboardUtils::init_atoms(Display *display)
{
    XA_NULL = XInternAtom(display, CLIPBOARD_ATOM_STR_NULL, False);
    XA_TARGETS = XInternAtom(display, CLIPBOARD_ATOM_STR_TARGETS, False);
    XA_TIMESTAMP = XInternAtom(display, CLIPBOARD_ATOM_STR_TIMESTAMP, False);
    XA_ATOM_PAIR = XInternAtom(display, CLIPBOARD_ATOM_STR_ATOM_PAIR, False);
    XA_DELETE = XInternAtom(display, CLIPBOARD_ATOM_STR_DELETE, False);
    XA_INCR = XInternAtom(display, CLIPBOARD_ATOM_STR_INCR, False);
    XA_MULTIPLE = XInternAtom(display, CLIPBOARD_ATOM_STR_MULTIPLE, False);
    XA_SAVE_TARGETS = XInternAtom(display, CLIPBOARD_ATOM_STR_SAVE_TARGETS, False);

    XA_INSERT_PROPERTY = XInternAtom(display, CLIPBOARD_ATOM_STR_INSERT_PROPERTY, False);
    XA_INSERT_SELECTION = XInternAtom(display, CLIPBOARD_ATOM_STR_INSERT_SELECTION, False);
    XA_MANAGER = XInternAtom(display, CLIPBOARD_ATOM_STR_MANAGER, False);
    XA_CLIPBOARD_MANAGER = XInternAtom(display, CLIPBOARD_ATOM_STR_CLIPBOARD_MANAGER, False);
    XA_CLIPBOARD = XInternAtom(display, CLIPBOARD_ATOM_STR_CLIPBOARD, False);

    KLOG_DEBUG("TARGETS: %lu, INCR: %lu, MULTIPLE: %lu, SAVE_TARGETS: %lu.",
               XA_TARGETS, XA_INCR, XA_MULTIPLE, XA_SAVE_TARGETS);
}

void ClipboardUtils::init_selection_max_size(Display *display)
{
    if (SELECTION_MAX_SIZE > 0)
        return;

    unsigned long max_request_size;

    max_request_size = XExtendedMaxRequestSize(display);
    if (max_request_size == 0)
        max_request_size = XMaxRequestSize(display);

    SELECTION_MAX_SIZE = max_request_size - 100;
    if (SELECTION_MAX_SIZE > 262144)
        SELECTION_MAX_SIZE = 262144;
}

bool ClipboardUtils::is_valid_target_in_save_targets(Atom target)
{
    if (target == XA_TARGETS ||
        target == XA_MULTIPLE ||
        target == XA_DELETE ||
        target == XA_INSERT_PROPERTY ||
        target == XA_INSERT_SELECTION ||
        target == XA_PIXMAP)
    {
        return false;
    }

    return true;
}

int ClipboardUtils::bytes_per_item(int format)
{
    switch (format)
    {
    case 8:
        return sizeof(char);
    case 16:
        return sizeof(short);
    case 32:
        return sizeof(long);
    default:
        break;
    }

    return 0;
}

static Bool
timestamp_predicate(Display *display,
                    XEvent *xevent,
                    XPointer arg)
{
    TimeStampInfo *info = (TimeStampInfo *)arg;

    if (xevent->type == PropertyNotify &&
        xevent->xproperty.window == info->window &&
        xevent->xproperty.atom == info->timestamp_prop_atom)
        return True;

    return False;
}

Time ClipboardUtils::get_server_time(Display *display,
                                     Window window)
{
    unsigned char c = 'a';
    XEvent xevent;
    TimeStampInfo info;

    info.timestamp_prop_atom = XInternAtom(display, "_TIMESTAMP_PROP", False);
    info.window = window;

    XChangeProperty(display, window,
                    info.timestamp_prop_atom, info.timestamp_prop_atom,
                    8, PropModeReplace, &c, 1);

    XIfEvent(display, &xevent,
             timestamp_predicate, (XPointer)&info);

    return xevent.xproperty.time;
}

void ClipboardUtils::change_window_filter(Window window,
                                          FilterChangeType type,
                                          GdkFilterFunc filter_func,
                                          void *filter_user)
{
    GdkDisplay *gdkdisplay = gdk_display_get_default();
    GdkWindow *gdkwin = gdk_x11_window_lookup_for_display(gdkdisplay, window);

    switch (type)
    {
    case FILTER_CHANGE_ADD:
    {
        if (gdkwin == NULL)
        {
            gdkwin = gdk_x11_window_foreign_new_for_display(gdkdisplay, window);
        }
        else
        {
            g_object_ref(gdkwin);
        }

        gdk_window_add_filter(gdkwin,
                              filter_func,
                              filter_user);
    }
    break;
    case FILTER_CHANGE_REMOVE:
    {
        if (gdkwin == NULL)
        {
            return;
        }
        gdk_window_remove_filter(gdkwin,
                                 filter_func,
                                 filter_user);

        g_object_unref(gdkwin);
    }
    break;
    default:
        break;
    }
}

void ClipboardUtils::response_selection_request(Display *display, XEvent *xev, bool success)
{
    KLOG_DEBUG("requestor: %u, success: %d.", xev->xselectionrequest.requestor, success);

    XSelectionEvent notify;

    notify.type = SelectionNotify;
    notify.serial = 0;
    notify.send_event = True;
    notify.display = xev->xselectionrequest.display;
    notify.requestor = xev->xselectionrequest.requestor;
    notify.selection = xev->xselectionrequest.selection;
    notify.target = xev->xselectionrequest.target;
    notify.property = success ? xev->xselectionrequest.property : None;
    notify.time = xev->xselectionrequest.time;

    GdkDisplay *gdkdisplay = gdk_display_get_default();
    gdk_x11_display_error_trap_push(gdkdisplay);

    XSendEvent(notify.display,
               notify.requestor,
               False,
               NoEventMask,
               (XEvent *)&notify);
    XSync(display, False);

    gdk_x11_display_error_trap_pop_ignored(gdkdisplay);
}

bool ClipboardUtils::get_window_property_group(Display *display,
                                               Window window,
                                               Atom property,
                                               Bool is_delete,
                                               Atom req_type,
                                               WindowPropertyGroup *prop_group)
{
    int result = XGetWindowProperty(display,
                                    window,
                                    property,
                                    0, 0x1FFFFFFF, is_delete, req_type,
                                    &prop_group->type, &prop_group->format, &prop_group->nitems, &prop_group->remaining,
                                    (unsigned char **)&prop_group->data);

    if (result != Success)
    {
        KLOG_WARNING("Failed window: %lu, property: <%lu, %s>.", window, property, XGetAtomName(display, property));
        return false;
    }

    KLOG_DEBUG("Success window: %lu, property: <%lu, %s>.", window, property, XGetAtomName(display, property));
    return true;
}

}  // namespace Kiran