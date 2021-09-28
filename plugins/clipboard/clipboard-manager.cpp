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

#include "plugins/clipboard/clipboard-manager.h"

namespace Kiran
{
ClipboardManager::ClipboardManager()
{
}

ClipboardManager *ClipboardManager::instance_ = nullptr;

void ClipboardManager::global_init()
{
    instance_ = new ClipboardManager;
    instance_->init();
}

void ClipboardManager::global_deinit()
{
    instance_->deinit();
    delete instance_;
}

GdkFilterReturn
ClipboardManager::event_filter(GdkXEvent *xevent,
                               GdkEvent *event,
                               ClipboardManager *manager)
{
    if (manager->process_event((XEvent *)xevent))
    {
        return GDK_FILTER_REMOVE;
    }
    else
    {
        return GDK_FILTER_CONTINUE;
    }
}

void ClipboardManager::init()
{
    KLOG_PROFILE();

    display_ = GDK_DISPLAY_XDISPLAY(gdk_display_get_default());

    init_atoms(display_);
    init_selection_max_size(display_);

    if (XGetSelectionOwner(display_, XA_CLIPBOARD_MANAGER))
    {
        KLOG_WARNING("Clipboard manager is already running.");
        return;
    }

    requestor_ = None;
    window_ = XCreateSimpleWindow(display_,
                                  DefaultRootWindow(display_),
                                  0, 0, 10, 10, 0,
                                  WhitePixel(display_, DefaultScreen(display_)),
                                  WhitePixel(display_, DefaultScreen(display_)));

    change_window_filter(window_, FILTER_CHANGE_ADD);
    XSelectInput(display_, window_, PropertyChangeMask);
    timestamp_ = get_server_time(display_, window_);

    XSetSelectionOwner(display_,
                       XA_CLIPBOARD_MANAGER,
                       window_,
                       timestamp_);

    if (XGetSelectionOwner(display_, XA_CLIPBOARD_MANAGER) == window_)
    {
        send_client_message();
    }
    else
    {
        KLOG_WARNING("Clipboard manager lost selection owner.");
        change_window_filter(window_, FILTER_CHANGE_REMOVE);
    }

    KLOG_WARNING("SelectionMaxSize: %lu, window: %lu.", SELECTION_MAX_SIZE, window_);

    return;
}

void ClipboardManager::deinit()
{
    change_window_filter(window_, FILTER_CHANGE_REMOVE);

    XDestroyWindow(display_, window_);

    clear_conversions();

    clear_contents();
}

void ClipboardManager::clear_contents()
{
    for (auto it = contents_.begin(); it != contents_.end(); it++)
    {
        TargetData *tdata = (TargetData *)(*it);
        if (tdata != NULL)
        {
            delete tdata;
        }
    }

    contents_.clear();
}

void ClipboardManager::clear_conversions()
{
    for (auto it = conversions_.begin(); it != conversions_.end(); it++)
    {
        IncrConversion *rdata = (IncrConversion *)(*it);
        if (rdata != NULL)
        {
            delete rdata;
        }
    }

    conversions_.clear();
}

void ClipboardManager::send_client_message()
{
    KLOG_PROFILE();

    XClientMessageEvent xev;
    xev.type = ClientMessage;
    xev.window = DefaultRootWindow(display_);
    xev.message_type = XA_MANAGER;
    xev.format = 32;
    xev.data.l[0] = timestamp_;
    xev.data.l[1] = XA_CLIPBOARD_MANAGER;
    xev.data.l[2] = window_;
    xev.data.l[3] = 0;
    xev.data.l[4] = 0;

    XSendEvent(display_,
               DefaultRootWindow(display_),
               False,
               StructureNotifyMask,
               (XEvent *)&xev);
}

void ClipboardManager::save_target_data(TargetData *tdata)
{
    WindowPropRetStruct prop_ret;
    bool ret = get_window_property_return_struct(display_,
                                                 window_,
                                                 tdata->target,
                                                 True, AnyPropertyType,
                                                 &prop_ret);
    RETURN_IF_FALSE(ret);

    if (prop_ret.type == None)
    {
        KLOG_DEBUG("TargetData type None.");

        contents_.remove(tdata);
        delete tdata;
    }
    else if (prop_ret.type == XA_INCR)
    {
        KLOG_DEBUG("TargetData type INCR.");

        tdata->type = prop_ret.type;
        tdata->length = 0;

        if (prop_ret.data)
        {
            XFree(prop_ret.data);
        }
    }
    else
    {
        tdata->type = prop_ret.type;
        tdata->format = prop_ret.format;
        tdata->data = prop_ret.data;
        tdata->length = prop_ret.nitems * bytes_per_item(prop_ret.format);

        KLOG_DEBUG("TargetData type: %s, format: %d, length: %lu.",
                   XGetAtomName(display_, tdata->target),
                   tdata->format, tdata->length);
    }
}

void ClipboardManager::collect_incremental(IncrConversion *rdata)
{
    if (rdata->offset >= 0)
    {
        conversions_.push_front(rdata);
    }
    else
    {
        delete rdata;
    }
}

void ClipboardManager::convert_clipboard_target(IncrConversion *rdata)
{
    KLOG_DEBUG("Target: %s.", XGetAtomName(display_, rdata->target));

    if (rdata->target == XA_TARGETS)
    {
        int n_targets = contents_.size() + 2;
        Atom *targets = new Atom[n_targets];

        n_targets = 0;

        targets[n_targets++] = XA_TARGETS;
        targets[n_targets++] = XA_MULTIPLE;

        for (auto it = contents_.begin(); it != contents_.end(); it++)
        {
            targets[n_targets++] = (*it)->target;
        }

        XChangeProperty(display_, rdata->requestor,
                        rdata->property, XA_ATOM,
                        32, PropModeReplace,
                        (unsigned char *)targets, n_targets);

        delete[] targets;
    }
    else
    {
        auto it = std::find_if(contents_.begin(), contents_.end(), FindContentTarget(rdata->target));

        RETURN_IF_TRUE(it == contents_.end());

        TargetData *tdata = (TargetData *)(*it);
        if (tdata->type == XA_INCR)
        {
            rdata->property = None;
            return;
        }

        RETURN_IF_FALSE(bytes_per_item(tdata->format) > 0);

        unsigned long nitems;
        rdata->data = tdata;
        nitems = tdata->length / bytes_per_item(tdata->format);
        if (tdata->length <= SELECTION_MAX_SIZE)
        {
            XChangeProperty(display_, rdata->requestor,
                            rdata->property, tdata->type,
                            tdata->format, PropModeReplace,
                            tdata->data, nitems);
        }
        else
        {
            KLOG_DEBUG("Start incremental transfer target: %s, length: %lu.",
                       XGetAtomName(display_, rdata->target), tdata->length);

            XWindowAttributes atts;
            rdata->offset = 0;

            GdkDisplay *display = gdk_display_get_default();
            gdk_x11_display_error_trap_push(display);

            XGetWindowAttributes(display_, rdata->requestor, &atts);

            change_window_filter(rdata->requestor, FILTER_CHANGE_ADD);

            XSelectInput(display_,
                         rdata->requestor,
                         atts.your_event_mask | PropertyChangeMask);

            XChangeProperty(display_, rdata->requestor,
                            rdata->property, XA_INCR,
                            32, PropModeReplace,
                            (unsigned char *)&nitems, 1);

            XSync(display_, False);

            gdk_x11_display_error_trap_pop_ignored(display);
        }
    }
}

void ClipboardManager::change_window_filter(Window window,
                                            FilterChangeType type)
{
    GdkWindow *gdkwin;
    GdkDisplay *display;

    display = gdk_display_get_default();
    gdkwin = gdk_x11_window_lookup_for_display(display, window);

    switch (type)
    {
    case FILTER_CHANGE_ADD:
    {
        if (gdkwin == NULL)
        {
            gdkwin = gdk_x11_window_foreign_new_for_display(display, window);
        }
        else
        {
            g_object_ref(gdkwin);
        }
        gdk_window_add_filter(gdkwin,
                              (GdkFilterFunc)ClipboardManager::event_filter,
                              this);
    }
    break;
    case FILTER_CHANGE_REMOVE:
    {
        if (gdkwin == NULL)
        {
            return;
        }

        gdk_window_remove_filter(gdkwin,
                                 (GdkFilterFunc)ClipboardManager::event_filter,
                                 this);
        g_object_unref(gdkwin);
    }
    break;
    default:
        break;
    }
}

bool ClipboardManager::process_event(XEvent *xev)
{
    switch (xev->xany.type)
    {
    case DestroyNotify:
    {
        if (xev->xdestroywindow.window == requestor_)
        {
            KLOG_DEBUG("DestroyNotify window: %lu.", requestor_);

            clear_contents();
            change_window_filter(requestor_, FILTER_CHANGE_REMOVE);
            requestor_ = None;
        }
    }
    break;
    case SelectionClear:
    {
        RETURN_VAL_IF_TRUE(xev->xany.window != window_, false);

        if (xev->xselectionclear.selection == XA_CLIPBOARD_MANAGER)
        {
            KLOG_DEBUG("SelectionClear CLIPBOARD_MANAGER.");

            clear_contents();
            XSetSelectionOwner(display_, XA_CLIPBOARD, None, time_);
            return true;
        }

        if (xev->xselectionclear.selection == XA_CLIPBOARD)
        {
            KLOG_DEBUG("SelectionClear CLIPBOARD.");

            clear_contents();
            change_window_filter(requestor_, FILTER_CHANGE_REMOVE);
            requestor_ = None;
            return true;
        }
    }
    break;
    case SelectionRequest:
    {
        RETURN_VAL_IF_TRUE(xev->xany.window != window_, false);

        if (xev->xselectionrequest.selection == XA_CLIPBOARD_MANAGER)
        {
            process_selectionrequestor_clipboard_manager(xev);
            return true;
        }

        if (xev->xselectionrequest.selection == XA_CLIPBOARD)
        {
            process_selectionrequestor_clipboard(xev);
            return true;
        }
    }
    break;
    case SelectionNotify:
    {
        RETURN_VAL_IF_TRUE(xev->xany.window != window_, false);

        RETURN_VAL_IF_TRUE(xev->xselection.selection != XA_CLIPBOARD, false);

        process_selectionnotify_clipboard(xev);
        return true;
    }
    break;
    case PropertyNotify:
    {
        if (xev->xproperty.state == PropertyNewValue)
        {
            return receive_incrementally(xev);
        }

        if (xev->xproperty.state == PropertyDelete)
        {
            return send_incrementally(xev);
        }
    }
    break;
    default:
        break;
    }

    return false;
}

void ClipboardManager::process_selectionrequestor_clipboard_manager(XEvent *xev)
{
    KLOG_PROFILE("Requestor: %lu, target: %s.",
                 xev->xselectionrequest.requestor,
                 XGetAtomName(xev->xselectionrequest.display, xev->xselectionrequest.target));

    if (xev->xselectionrequest.target == XA_TIMESTAMP)
    {
        XChangeProperty(display_,
                        xev->xselectionrequest.requestor,
                        xev->xselectionrequest.property,
                        XA_INTEGER, 32, PropModeReplace,
                        (unsigned char *)&timestamp_, 1);
        response_selection_request(xev, true);
    }
    else if (xev->xselectionrequest.target == XA_TARGETS)
    {
        Atom targets[3] = {XA_TARGETS, XA_TIMESTAMP, XA_SAVE_TARGETS};

        XChangeProperty(display_,
                        xev->xselectionrequest.requestor,
                        xev->xselectionrequest.property,
                        XA_ATOM, 32, PropModeReplace,
                        (unsigned char *)targets, 3);

        response_selection_request(xev, true);
    }
    else if (xev->xselectionrequest.target == XA_SAVE_TARGETS)
    {
        selection_request_save_targets(xev);
    }
    else
    {
        response_selection_request(xev, false);
    }
}

void ClipboardManager::process_selectionrequestor_clipboard(XEvent *xev)
{
    KLOG_PROFILE("Requestor: %lu, target: %s.",
                 xev->xselectionrequest.requestor,
                 XGetAtomName(xev->xselectionrequest.display, xev->xselectionrequest.target));

    if (xev->xselectionrequest.target == XA_MULTIPLE)
    {
        selection_request_clipboard_multiple(xev);
    }
    else
    {
        selection_request_clipboard_single(xev);
    }
}

void ClipboardManager::process_selectionnotify_clipboard(XEvent *xev)
{
    KLOG_PROFILE("Property: %s.", XGetAtomName(xev->xselection.display, xev->xselection.property));

    if (xev->xselection.property == XA_TARGETS)
    {
        WindowPropRetStruct prop_ret;

        bool ret = get_window_property_return_struct(xev->xselection.display,
                                                     xev->xselection.requestor,
                                                     xev->xselection.property,
                                                     True, XA_ATOM,
                                                     &prop_ret);

        RETURN_IF_FALSE(ret);

        save_targets((Atom *)prop_ret.data, prop_ret.nitems);
    }
    else if (xev->xselection.property == XA_MULTIPLE)
    {
        selection_notify_clipboard_multiple(xev);
    }
    else if (xev->xselection.property == None)
    {
        response_manager_save_targets(false);
        change_window_filter(requestor_, FILTER_CHANGE_REMOVE);
        requestor_ = None;
    }
}

void ClipboardManager::save_targets(Atom *targets, unsigned long nitems)
{
    KLOG_PROFILE("Nitems: %lu.", nitems);

    RETURN_IF_FALSE(targets);

    if (nitems == 0)
    {
        XFree(targets);
        return;
    }

    Atom *multiple = new Atom[2 * nitems];

    unsigned long nout = 0;
    for (unsigned long i = 0; i < nitems; i++)
    {
        if (is_valid_target_in_save_targets(targets[i]))
        {
            TargetData *tdata = new TargetData();
            tdata->target = targets[i];
            contents_.push_front(tdata);

            multiple[nout++] = targets[i];
            multiple[nout++] = targets[i];

            KLOG_DEBUG("Item: %d, target: %s.", i, XGetAtomName(display_, targets[i]));
        }
    }

    if (nout > 0)
    {
        XChangeProperty(display_, window_,
                        XA_MULTIPLE, XA_ATOM_PAIR,
                        32, PropModeReplace,
                        (unsigned char *)multiple, nout);

        XConvertSelection(display_, XA_CLIPBOARD,
                          XA_MULTIPLE, XA_MULTIPLE,
                          window_, time_);
    }

    XFree(targets);
    delete[] multiple;
}

bool ClipboardManager::receive_incrementally(XEvent *xev)
{
    RETURN_VAL_IF_TRUE(xev->xproperty.window != window_, false);

    auto it_target = std::find_if(contents_.begin(), contents_.end(), FindContentTarget(xev->xproperty.atom));

    RETURN_VAL_IF_TRUE(it_target == contents_.end(), false);

    TargetData *tdata = (TargetData *)(*it_target);

    RETURN_VAL_IF_TRUE(tdata->type != XA_INCR, false);

    WindowPropRetStruct prop_ret;
    bool ret = get_window_property_return_struct(xev->xproperty.display,
                                                 xev->xproperty.window,
                                                 xev->xproperty.atom,
                                                 True, AnyPropertyType,
                                                 &prop_ret);

    RETURN_VAL_IF_FALSE(ret, false);

    unsigned long length = prop_ret.nitems * bytes_per_item(prop_ret.format);

    KLOG_DEBUG("Receive type: %s, length: %lu, nitems: %lu, format: %d.",
               XGetAtomName(xev->xproperty.display, prop_ret.type),
               length, prop_ret.nitems, prop_ret.format);

    if (length == 0)
    {
        tdata->type = prop_ret.type;
        tdata->format = prop_ret.format;

        auto it_type = std::find_if(contents_.begin(), contents_.end(), FindContentType(XA_INCR));
        if (it_type == contents_.end())
        {
            KLOG_DEBUG("All incremental transfers done, property: %s, type: %s.",
                       XGetAtomName(xev->xproperty.display, xev->xproperty.atom),
                       XGetAtomName(xev->xproperty.display, prop_ret.type));

            response_manager_save_targets(true);
            requestor_ = None;
        }

        XFree(prop_ret.data);
    }
    else
    {
        if (!tdata->data)
        {
            tdata->data = prop_ret.data;
            tdata->length = length;
        }
        else
        {
            tdata->data = (unsigned char *)realloc(tdata->data, tdata->length + length + 1);
            memcpy(tdata->data + tdata->length, prop_ret.data, length + 1);
            tdata->length += length;
            XFree(prop_ret.data);
        }
    }

    return true;
}

bool ClipboardManager::send_incrementally(XEvent *xev)
{
    auto it = std::find_if(conversions_.begin(), conversions_.end(), FindConversionRequestor(xev));
    RETURN_VAL_IF_TRUE(it == conversions_.end(), false);

    IncrConversion *rdata;
    unsigned long length;
    unsigned long nitems;
    unsigned char *data;

    rdata = (IncrConversion *)(*it);

    RETURN_VAL_IF_FALSE(bytes_per_item(rdata->data->format) > 0, false);

    data = rdata->data->data + rdata->offset;
    length = rdata->data->length - rdata->offset;

    if (length > SELECTION_MAX_SIZE)
    {
        length = SELECTION_MAX_SIZE;
    }

    KLOG_DEBUG("Total_length:%lu, offset: %d, length: %lu.",
               rdata->data->length, rdata->offset, length);

    rdata->offset += length;
    nitems = length / bytes_per_item(rdata->data->format);

    XChangeProperty(display_, rdata->requestor,
                    rdata->property, rdata->data->type,
                    rdata->data->format, PropModeAppend,
                    data, nitems);

    if (length == 0)
    {
        KLOG_DEBUG("Send all incr done, property: %s, target: %s, length:%lu.",
                   XGetAtomName(display_, rdata->property),
                   XGetAtomName(display_, rdata->data->target),
                   rdata->data->length);

        change_window_filter(rdata->requestor, FILTER_CHANGE_REMOVE);

        conversions_.remove(rdata);
        delete rdata;
    }

    return true;
}

void ClipboardManager::response_manager_save_targets(bool success)
{
    KLOG_DEBUG("Send CLIPBOARD_MANAGER SAVE_TARGETS, requestor: %lu, success: %d.", requestor_, success);

    XSelectionEvent notify;
    GdkDisplay *display;

    notify.type = SelectionNotify;
    notify.serial = 0;
    notify.send_event = True;
    notify.display = display_;
    notify.requestor = requestor_;
    notify.selection = XA_CLIPBOARD_MANAGER;
    notify.target = XA_SAVE_TARGETS;
    notify.property = success ? property_ : None;
    notify.time = time_;

    display = gdk_display_get_default();
    gdk_x11_display_error_trap_push(display);

    XSendEvent(display_,
               requestor_,
               False,
               NoEventMask,
               (XEvent *)&notify);
    XSync(display_, False);

    gdk_x11_display_error_trap_pop_ignored(display);
}

void ClipboardManager::response_selection_request(XEvent *xev, bool success)
{
    KLOG_DEBUG("Send SelectionNotify, requestor: %lu, success: %d.", xev->xselectionrequest.display, success);

    XSelectionEvent notify;
    GdkDisplay *display;

    notify.type = SelectionNotify;
    notify.serial = 0;
    notify.send_event = True;
    notify.display = xev->xselectionrequest.display;
    notify.requestor = xev->xselectionrequest.requestor;
    notify.selection = xev->xselectionrequest.selection;
    notify.target = xev->xselectionrequest.target;
    notify.property = success ? xev->xselectionrequest.property : None;
    notify.time = xev->xselectionrequest.time;

    display = gdk_display_get_default();
    gdk_x11_display_error_trap_push(display);

    XSendEvent(xev->xselectionrequest.display,
               xev->xselectionrequest.requestor,
               False,
               NoEventMask,
               (XEvent *)&notify);
    XSync(display_, False);

    gdk_x11_display_error_trap_pop_ignored(display);
}

void ClipboardManager::selection_request_save_targets(XEvent *xev)
{
    KLOG_PROFILE();

    if (requestor_ != None || contents_.empty() == false)
    {
        response_selection_request(xev, false);
        return;
    }

    GdkDisplay *display = gdk_display_get_default();

    gdk_x11_display_error_trap_push(display);

    change_window_filter(xev->xselectionrequest.requestor, FILTER_CHANGE_ADD);
    XSelectInput(display_, xev->xselectionrequest.requestor, StructureNotifyMask);
    XSync(display_, False);

    if (gdk_x11_display_error_trap_pop(display) != Success)
    {
        return;
    }

    WindowPropRetStruct prop_ret;
    if (xev->xselectionrequest.property != None)
    {
        bool ret = get_window_property_return_struct(display_,
                                                     xev->xselectionrequest.requestor,
                                                     xev->xselectionrequest.property,
                                                     False, XA_ATOM,
                                                     &prop_ret);

        RETURN_IF_FALSE(ret);
    }

    requestor_ = xev->xselectionrequest.requestor;
    property_ = xev->xselectionrequest.property;
    time_ = xev->xselectionrequest.time;

    if (prop_ret.type == None)
    {
        XConvertSelection(display_, XA_CLIPBOARD,
                          XA_TARGETS, XA_TARGETS,
                          window_, time_);
    }
    else
    {
        save_targets((Atom *)prop_ret.data, prop_ret.nitems);
    }
}

void ClipboardManager::selection_notify_clipboard_multiple(XEvent *xev)
{
    KLOG_PROFILE();

    std::list<TargetData *> tmp_contents = contents_;

    for (auto it_target = tmp_contents.begin(); it_target != tmp_contents.end(); it_target++)
    {
        save_target_data((TargetData *)(*it_target));
    }

    tmp_contents.clear();

    time_ = xev->xselection.time;
    XSetSelectionOwner(display_,
                       XA_CLIPBOARD,
                       window_,
                       time_);

    if (property_ != None)
    {
        XChangeProperty(display_, requestor_,
                        property_, XA_ATOM,
                        32, PropModeReplace,
                        (unsigned char *)&XA_NULL, 1);
    }

    auto it_type = std::find_if(contents_.begin(), contents_.end(), FindContentType(XA_INCR));
    if (it_type == contents_.end())
    {
        KLOG_DEBUG("All transfers done");

        response_manager_save_targets(true);
        change_window_filter(requestor_, FILTER_CHANGE_REMOVE);
        requestor_ = None;
    }
}

void ClipboardManager::selection_request_clipboard_multiple(XEvent *xev)
{
    WindowPropRetStruct prop_ret;
    int ret = get_window_property_return_struct(xev->xselectionrequest.display,
                                                xev->xselectionrequest.requestor,
                                                xev->xselectionrequest.property,
                                                False, XA_ATOM_PAIR,
                                                &prop_ret);

    RETURN_IF_FALSE(ret);

    if (prop_ret.type != XA_ATOM_PAIR || prop_ret.nitems == 0)
    {
        if (prop_ret.data)
        {
            XFree(prop_ret.data);
        }
        return;
    }

    IncrConversion *rdata;
    std::list<IncrConversion *> conversions;
    std::list<IncrConversion *>::iterator it;

    Atom *multiple = (Atom *)prop_ret.data;
    int nitems = 0;

    for (unsigned long i = 0; i < prop_ret.nitems; i += 2)
    {
        rdata = new IncrConversion();
        rdata->requestor = xev->xselectionrequest.requestor;
        rdata->target = multiple[i];
        rdata->property = multiple[i + 1];
        conversions.push_front(rdata);

        convert_clipboard_target(rdata);

        KLOG_DEBUG("Multiple target: %s, property: %s.",
                   XGetAtomName(xev->xselectionrequest.display, rdata->target),
                   XGetAtomName(xev->xselectionrequest.display, rdata->property));
    }

    for (it = conversions.begin(); it != conversions.end(); it++)
    {
        rdata = (IncrConversion *)(*it);
        multiple[nitems++] = rdata->target;
        multiple[nitems++] = rdata->property;
    }

    XChangeProperty(xev->xselectionrequest.display,
                    xev->xselectionrequest.requestor,
                    xev->xselectionrequest.property,
                    XA_ATOM_PAIR, 32, PropModeReplace,
                    (unsigned char *)multiple, nitems);

    response_selection_request(xev, true);

    for (it = conversions.begin(); it != conversions.end(); it++)
    {
        collect_incremental((IncrConversion *)(*it));
    }

    conversions.clear();

    if (prop_ret.data)
    {
        XFree(prop_ret.data);
    }
}

void ClipboardManager::selection_request_clipboard_single(XEvent *xev)
{
    IncrConversion *rdata = new IncrConversion();
    rdata->requestor = xev->xselectionrequest.requestor;
    rdata->target = xev->xselectionrequest.target;
    rdata->property = xev->xselectionrequest.property;

    KLOG_DEBUG("Target: %s, property: %s.",
               XGetAtomName(xev->xselectionrequest.display, rdata->target),
               XGetAtomName(xev->xselectionrequest.display, rdata->property));

    convert_clipboard_target(rdata);

    if (rdata->property == None)
    {
        response_selection_request(xev, false);
    }
    else
    {
        response_selection_request(xev, true);
    }

    collect_incremental(rdata);
}

}  // namespace Kiran