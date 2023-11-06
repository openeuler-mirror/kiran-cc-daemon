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
ClipboardManager *ClipboardManager::instance_ = nullptr;

ClipboardManager::ClipboardManager()
{
}

ClipboardManager::~ClipboardManager()
{
}

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
    if (manager->process_event(static_cast<XEvent *>(xevent)))
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
    this->display_ = GDK_DISPLAY_XDISPLAY(gdk_display_get_default());

    ClipboardUtils::init_atoms(this->display_);
    ClipboardUtils::init_selection_max_size(this->display_);

    if (XGetSelectionOwner(this->display_, XA_CLIPBOARD_MANAGER))
    {
        KLOG_WARNING_CLIPBOARD("Clipboard manager is already running.");
        return;
    }

    this->requestor_ = None;
    this->window_ = XCreateSimpleWindow(this->display_,
                                        DefaultRootWindow(this->display_),
                                        0, 0, 10, 10, 0,
                                        WhitePixel(this->display_, DefaultScreen(this->display_)),
                                        WhitePixel(this->display_, DefaultScreen(this->display_)));

    this->clipboard_data_.init();
    this->clipboard_.init(this->display_,
                          this->window_,
                          (GdkFilterFunc)ClipboardManager::event_filter,
                          this,
                          &this->clipboard_data_);

    ClipboardUtils::change_window_filter(this->window_,
                                         FILTER_CHANGE_ADD,
                                         (GdkFilterFunc)ClipboardManager::event_filter,
                                         this);
    XSelectInput(this->display_, this->window_, PropertyChangeMask);
    this->timestamp_ = ClipboardUtils::get_server_time(this->display_, this->window_);

    XSetSelectionOwner(this->display_,
                       XA_CLIPBOARD_MANAGER,
                       this->window_,
                       this->timestamp_);

    if (XGetSelectionOwner(this->display_, XA_CLIPBOARD_MANAGER) == this->window_)
    {
        this->send_client_message();
    }
    else
    {
        KLOG_WARNING_CLIPBOARD("Clipboard manager lost selection owner.");
        ClipboardUtils::change_window_filter(this->window_,
                                             FILTER_CHANGE_REMOVE,
                                             (GdkFilterFunc)ClipboardManager::event_filter,
                                             this);
    }

    KLOG_DEBUG_CLIPBOARD("Init SelectionMaxSize is %lu, window is %lu.", SELECTION_MAX_SIZE, window_);

    return;
}

void ClipboardManager::deinit()
{
    if (this->window_ != None)
    {
        ClipboardUtils::change_window_filter(this->window_,
                                             FILTER_CHANGE_REMOVE,
                                             (GdkFilterFunc)ClipboardManager::event_filter,
                                             this);

        XDestroyWindow(this->display_, this->window_);
        this->window_ = None;
    }

    this->clipboard_.deinit();
    this->clipboard_data_.deinit();
}

void ClipboardManager::clear_requestor()
{
    this->clipboard_data_.clear_contents();
    ClipboardUtils::change_window_filter(this->requestor_,
                                         FILTER_CHANGE_REMOVE,
                                         (GdkFilterFunc)ClipboardManager::event_filter,
                                         this);
    this->requestor_ = None;
}

void ClipboardManager::clear_clipboard_owner()
{
    this->clipboard_data_.clear_contents();
    XSetSelectionOwner(this->display_, XA_CLIPBOARD, None, this->time_);
}

bool ClipboardManager::process_event(XEvent *xev)
{
    switch (xev->xany.type)
    {
    case DestroyNotify:
    {
        if (xev->xdestroywindow.window == this->requestor_)
        {
            KLOG_DEBUG_CLIPBOARD("DestroyNotify window: %lu.", xev->xdestroywindow.window);
            this->clear_requestor();
        }
    }
    break;
    case SelectionClear:
    {
        RETURN_VAL_IF_TRUE(xev->xany.window != this->window_, false);

        if (xev->xselectionclear.selection == XA_CLIPBOARD_MANAGER)
        {
            KLOG_DEBUG_CLIPBOARD("SelectionClear XA_CLIPBOARD_MANAGER");
            this->clear_clipboard_owner();
            return true;
        }

        if (xev->xselectionclear.selection == XA_CLIPBOARD)
        {
            KLOG_DEBUG_CLIPBOARD("SelectionClear XA_CLIPBOARD");
            this->clear_requestor();
            return true;
        }
    }
    break;
    case SelectionRequest:
    {
        RETURN_VAL_IF_TRUE(xev->xany.window != this->window_, false);

        if (xev->xselectionrequest.selection == XA_CLIPBOARD_MANAGER)
        {
            KLOG_DEBUG_CLIPBOARD("SelectionRequest XA_CLIPBOARD_MANAGER");
            this->selection_request_clipboard_manager(xev);
            return true;
        }

        if (xev->xselectionrequest.selection == XA_CLIPBOARD)
        {
            KLOG_DEBUG_CLIPBOARD("SelectionRequest XA_CLIPBOARD");
            this->clipboard_.selection_request_clipboard(xev);
            return true;
        }
    }
    break;
    case SelectionNotify:
    {
        RETURN_VAL_IF_TRUE(xev->xany.window != this->window_, false);

        if (xev->xselection.selection == XA_CLIPBOARD)
        {
            KLOG_DEBUG_CLIPBOARD("SelectionNotify XA_CLIPBOARD");
            this->selection_notify(xev);
            return true;
        }
    }
    break;
    case PropertyNotify:
    {
        if (xev->xproperty.state == PropertyNewValue)
        {
            RETURN_VAL_IF_TRUE(xev->xany.window != this->window_, false);

            return this->receive_incrementally(xev);
        }

        if (xev->xproperty.state == PropertyDelete)
        {
            return this->clipboard_.send_incrementally(xev);
        }
    }
    break;
    default:
        break;
    }
    return false;
}

void ClipboardManager::send_client_message()
{
    KLOG_DEBUG_CLIPBOARD("Send client message.");

    XClientMessageEvent xev;
    xev.type = ClientMessage;
    xev.window = DefaultRootWindow(this->display_);
    xev.message_type = XA_MANAGER;
    xev.format = 32;
    xev.data.l[0] = this->timestamp_;
    xev.data.l[1] = XA_CLIPBOARD_MANAGER;
    xev.data.l[2] = this->window_;
    xev.data.l[3] = 0;
    xev.data.l[4] = 0;

    XSendEvent(this->display_,
               DefaultRootWindow(this->display_),
               False,
               StructureNotifyMask,
               (XEvent *)&xev);
}

void ClipboardManager::selection_request_clipboard_manager(XEvent *xev)
{
    if (xev->xselectionrequest.target == XA_SAVE_TARGETS)
    {
        this->selection_request_save_targets(xev);
    }
    else if (xev->xselectionrequest.target == XA_TIMESTAMP)
    {
        XChangeProperty(this->display_,
                        xev->xselectionrequest.requestor,
                        xev->xselectionrequest.property,
                        XA_INTEGER, 32, PropModeReplace,
                        (unsigned char *)&this->timestamp_, 1);

        ClipboardUtils::response_selection_request(this->display_, xev, true);
    }
    else if (xev->xselectionrequest.target == XA_TARGETS)
    {
        Atom targets[3] = {XA_TARGETS, XA_TIMESTAMP, XA_SAVE_TARGETS};

        XChangeProperty(display_,
                        xev->xselectionrequest.requestor,
                        xev->xselectionrequest.property,
                        XA_ATOM, 32, PropModeReplace,
                        reinterpret_cast<unsigned char *>(targets), 3);

        ClipboardUtils::response_selection_request(this->display_, xev, true);
    }
    else
    {
        ClipboardUtils::response_selection_request(this->display_, xev, false);
    }
}

void ClipboardManager::selection_request_save_targets(XEvent *xev)
{
    if (this->requestor_ != None || !(this->clipboard_data_.is_contents_empty()))
    {
        // We're in the middle of a conversion request, or own the CLIPBOARD already
        KLOG_DEBUG_CLIPBOARD("The requestor is %ld.", this->requestor_);
        ClipboardUtils::response_selection_request(this->display_, xev, false);
        return;
    }

    GdkDisplay *gdkdisplay = gdk_display_get_default();

    gdk_x11_display_error_trap_push(gdkdisplay);

    ClipboardUtils::change_window_filter(xev->xselectionrequest.requestor,
                                         FILTER_CHANGE_ADD,
                                         (GdkFilterFunc)ClipboardManager::event_filter,
                                         this);
    XSelectInput(this->display_, xev->xselectionrequest.requestor, StructureNotifyMask);
    XSync(this->display_, False);

    if (gdk_x11_display_error_trap_pop(gdkdisplay) != Success)
    {
        KLOG_ERROR_CLIPBOARD("Save targets failed, requestor is %lu.", xev->xselectionrequest.requestor);
        return;
    }

    WindowPropertyGroup prop_group;
    if (xev->xselectionrequest.property != None)
    {
        bool ret = ClipboardUtils::get_window_property_group(this->display_,
                                                             xev->xselectionrequest.requestor,
                                                             xev->xselectionrequest.property,
                                                             False, XA_ATOM,
                                                             &prop_group);

        RETURN_IF_FALSE(ret);
    }

    this->requestor_ = xev->xselectionrequest.requestor;
    this->property_ = xev->xselectionrequest.property;
    this->time_ = xev->xselectionrequest.time;

    if (prop_group.type == None)
    {
        XConvertSelection(this->display_, XA_CLIPBOARD,
                          XA_TARGETS, XA_TARGETS,
                          this->window_, this->time_);
        KLOG_DEBUG_CLIPBOARD("XConvertSelection XA_TARGETS.");
    }
    else
    {
        KLOG_DEBUG_CLIPBOARD("Multiple nitems is %lu.", prop_group.nitems);
        save_targets(reinterpret_cast<Atom *>(prop_group.data), prop_group.nitems);
    }
}

void ClipboardManager::save_targets(Atom *targets, unsigned long nitems)
{
    RETURN_IF_FALSE(targets != nullptr && nitems != 0);

    Atom *multiple = new Atom[2 * nitems];
    if (multiple == nullptr)
    {
        KLOG_ERROR_CLIPBOARD("Malloc memory failed.");
        return;
    }

    unsigned long nout = 0;
    for (unsigned long i = 0; i < nitems; i++)
    {
        if (ClipboardUtils::is_valid_target_in_save_targets(targets[i]))
        {
            this->clipboard_data_.add_target_data(targets[i]);

            multiple[nout++] = targets[i];
            multiple[nout++] = targets[i];
            KLOG_DEBUG_CLIPBOARD("Num: %lu, target: %lu.", i, targets[i]);
        }
    }

    if (nout > 0)
    {
        XChangeProperty(this->display_, this->window_,
                        XA_MULTIPLE, XA_ATOM_PAIR,
                        32, PropModeReplace,
                        reinterpret_cast<unsigned char *>(multiple), nout);

        XConvertSelection(this->display_, XA_CLIPBOARD,
                          XA_MULTIPLE, XA_MULTIPLE,
                          this->window_, time_);
    }

    delete[] multiple;
}

void ClipboardManager::selection_notify(XEvent *xev)
{
    if (xev->xselection.property == XA_TARGETS)
    {
        WindowPropertyGroup prop_group;
        bool ret = ClipboardUtils::get_window_property_group(xev->xselection.display,
                                                             xev->xselection.requestor,
                                                             xev->xselection.property,
                                                             True, XA_ATOM,
                                                             &prop_group);

        RETURN_IF_FALSE(ret);

        KLOG_DEBUG_CLIPBOARD("Multiple nitems is %lu.", prop_group.nitems);
        this->save_targets(reinterpret_cast<Atom *>(prop_group.data), prop_group.nitems);
    }
    else if (xev->xselection.property == XA_MULTIPLE)
    {
        this->save_multiple_property(xev);
    }
    else if (xev->xselection.property == None)
    {
        KLOG_DEBUG_CLIPBOARD("Property none.");
        this->response_manager_save_targets(false);
        this->clear_requestor();
    }
}

void ClipboardManager::save_multiple_property(XEvent *xev)
{
    KLOG_DEBUG_CLIPBOARD("Save multiple property.");
    this->clipboard_data_.save_targets_data(this->display_, this->window_);

    this->time_ = xev->xselection.time;
    XSetSelectionOwner(this->display_,
                       XA_CLIPBOARD,
                       this->window_,
                       this->time_);

    if (this->property_ != None)
    {
        XChangeProperty(this->display_, this->requestor_,
                        this->property_, XA_ATOM,
                        32, PropModeReplace,
                        (unsigned char *)&XA_NULL, 1);
    }

    bool find_incr = this->clipboard_data_.is_exist_type(XA_INCR);
    if (!find_incr)
    {
        KLOG_DEBUG_CLIPBOARD("All transfers done.");
        this->response_manager_save_targets(true);
        ClipboardUtils::change_window_filter(this->requestor_,
                                             FILTER_CHANGE_REMOVE,
                                             (GdkFilterFunc)ClipboardManager::event_filter,
                                             this);
        this->requestor_ = None;
    }
}

void ClipboardManager::response_manager_save_targets(bool success)
{
    XSelectionEvent notify;

    notify.type = SelectionNotify;
    notify.serial = 0;
    notify.send_event = True;
    notify.requestor = this->requestor_;
    notify.selection = XA_CLIPBOARD_MANAGER;
    notify.target = XA_SAVE_TARGETS;
    notify.property = success ? this->property_ : None;
    notify.time = this->time_;

    GdkDisplay *gdkdisplay = gdk_display_get_default();
    gdk_x11_display_error_trap_push(gdkdisplay);

    XSendEvent(this->display_,
               this->requestor_,
               False,
               NoEventMask,
               (XEvent *)&notify);
    XSync(this->display_, False);

    gdk_x11_display_error_trap_pop_ignored(gdkdisplay);
}

bool ClipboardManager::receive_incrementally(XEvent *xev)
{
    auto tdata = this->clipboard_data_.get_target_data_by_target(xev->xproperty.atom);

    RETURN_VAL_IF_TRUE(tdata == nullptr, false);

    RETURN_VAL_IF_TRUE(tdata->type != XA_INCR, false);

    WindowPropertyGroup prop_group;
    bool ret = ClipboardUtils::get_window_property_group(xev->xproperty.display,
                                                         xev->xproperty.window,
                                                         xev->xproperty.atom,
                                                         True, AnyPropertyType,
                                                         &prop_group);

    RETURN_VAL_IF_FALSE(ret, false);

    this->clipboard_data_.save_incremental_target_data(tdata, prop_group);

    if (prop_group.nitems == 0 ||
        ClipboardUtils::bytes_per_item(prop_group.format) == 0)
    {
        bool find_incr = this->clipboard_data_.is_exist_type(XA_INCR);
        if (!find_incr)
        {
            KLOG_DEBUG_CLIPBOARD("All incremental transfers done.");

            this->response_manager_save_targets(true);
            this->requestor_ = None;
        }
    }

    return true;
}

}  // namespace Kiran