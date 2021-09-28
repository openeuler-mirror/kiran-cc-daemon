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

void init_atoms(Display *display)
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
}

void init_selection_max_size(Display *display)
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

bool is_valid_target_in_save_targets(Atom target)
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

int bytes_per_item(int format)
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

Time get_server_time(Display *display,
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

bool get_window_property_return_struct(Display *display,
                                       Window window,
                                       Atom property,
                                       Bool is_delete,
                                       Atom req_type,
                                       WindowPropRetStruct *prop_ret)
{
    int retsult = XGetWindowProperty(display,
                                     window,
                                     property,
                                     0, 0x1FFFFFFF, is_delete, req_type,
                                     &prop_ret->type, &prop_ret->format, &prop_ret->nitems, &prop_ret->remaining,
                                     (unsigned char **)&prop_ret->data);

    if (retsult != Success)
    {
        if (prop_ret->data)
        {
            XFree(prop_ret->data);
        }

        KLOG_WARNING("Failed window: %lu, property: %s.", window, XGetAtomName(display, property));
        return false;
    }

    KLOG_DEBUG("Success window: %lu, property: %s.", window, XGetAtomName(display, property));
    return true;
}