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

#include "plugins/clipboard/clipboard.h"

namespace Kiran
{
Clipboard::Clipboard()
{
    this->clipboard_data_ = nullptr;
}

Clipboard::~Clipboard()
{
}

void Clipboard::init(Display* display,
                     Window window,
                     GdkFilterFunc filter_func,
                     void* user_data,
                     ClipboardData* clipboard_data)
{
    this->display_ = display;
    this->window_ = window;
    this->filter_func_ = filter_func;
    this->user_data_ = user_data;
    this->clipboard_data_ = clipboard_data;
}

void Clipboard::deinit()
{
    this->conversions_.clear();
}

bool Clipboard::send_incrementally(XEvent* xev)
{
    std::shared_ptr<IncrConversion> rdata;
    auto iter = this->conversions_.begin();
    for (; iter != this->conversions_.end(); iter++)
    {
        if ((*iter)->requestor == xev->xproperty.window &&
            (*iter)->property == xev->xproperty.atom)
        {
            rdata = (*iter);
            break;
        }
    }

    RETURN_VAL_IF_TRUE(rdata == nullptr, false);

    int bytes_per_item = ClipboardUtils::bytes_per_item(rdata->data->format);
    if (bytes_per_item == 0)
    {
        return false;
    }

    unsigned long nitems = 0;
    unsigned char* data = rdata->data->data + rdata->offset;
    unsigned long length = rdata->data->length - rdata->offset;

    if (length > SELECTION_MAX_SIZE)
    {
        // Fix the selelction max size
        length = SELECTION_MAX_SIZE;
    }

    rdata->offset += length;
    nitems = length / bytes_per_item;

    XChangeProperty(this->display_, rdata->requestor,
                    rdata->property, rdata->data->type,
                    rdata->data->format, PropModeAppend,
                    data, nitems);

    if (length <= 0)
    {
        KLOG_DEBUG("All incrementl data done, target: %lu.", rdata->target);
        // All incrementl data done.
        ClipboardUtils::change_window_filter(rdata->requestor,
                                             FILTER_CHANGE_REMOVE,
                                             this->filter_func_,
                                             this->user_data_);
        this->conversions_.erase(iter);
    }

    return true;
}

void Clipboard::selection_request_clipboard(XEvent* xev)
{
    if (xev->xselectionrequest.target == XA_MULTIPLE)
    {
        this->selection_request_clipboard_multiple(xev);
    }
    else
    {
        this->selection_request_clipboard_single(xev);
    }
}

void Clipboard::selection_request_clipboard_single(XEvent* xev)
{
    KLOG_PROFILE("");
    std::shared_ptr<IncrConversion> rdata = std::make_shared<IncrConversion>();
    rdata->requestor = xev->xselectionrequest.requestor;
    rdata->target = xev->xselectionrequest.target;
    rdata->property = xev->xselectionrequest.property;
    rdata->offset = INCR_OFFSET_NONE;

    convert_clipboard_target(rdata);

    if (rdata->property == None)
    {
        ClipboardUtils::response_selection_request(this->display_, xev, false);
    }
    else
    {
        ClipboardUtils::response_selection_request(this->display_, xev, true);
    }

    collect_incremental(rdata);
}

void Clipboard::selection_request_clipboard_multiple(XEvent* xev)
{
    KLOG_PROFILE("");
    WindowPropertyGroup prop_group;
    int ret = ClipboardUtils::get_window_property_group(xev->xselectionrequest.display,
                                                        xev->xselectionrequest.requestor,
                                                        xev->xselectionrequest.property,
                                                        False, XA_ATOM_PAIR,
                                                        &prop_group);

    if (!ret ||
        (prop_group.type != XA_ATOM_PAIR) ||
        (prop_group.nitems == 0))
    {
        return;
    }

    Atom* multiple = (Atom*)prop_group.data;

    std::vector<std::shared_ptr<IncrConversion>> conversions;

    for (unsigned long i = 0; i < prop_group.nitems; i += 2)
    {
        std::shared_ptr<IncrConversion> rdata = std::make_shared<IncrConversion>();
        rdata->requestor = xev->xselectionrequest.requestor;
        rdata->target = multiple[i];
        rdata->property = multiple[i + 1];
        rdata->data = nullptr;
        rdata->offset = INCR_OFFSET_NONE;
        conversions.push_back(rdata);

        this->convert_clipboard_target(rdata);

        KLOG_DEBUG("Multiple target: %lu, property: %lu.", rdata->target, rdata->property);
    }

    int nitems = 0;
    Atom* new_multiple = new Atom[2 * conversions.size()];
    if (new_multiple == nullptr)
    {
        KLOG_ERROR("Malloc memory failed.");
        conversions.clear();
        return;
    }

    for (auto iter : conversions)
    {
        new_multiple[nitems++] = iter->target;
        new_multiple[nitems++] = iter->property;
    }

    XChangeProperty(xev->xselectionrequest.display,
                    xev->xselectionrequest.requestor,
                    xev->xselectionrequest.property,
                    XA_ATOM_PAIR, 32, PropModeReplace,
                    (unsigned char*)new_multiple, nitems);

    ClipboardUtils::response_selection_request(this->display_, xev, true);

    for (auto iter : conversions)
    {
        this->collect_incremental(iter);
    }

    conversions.clear();
    if (new_multiple)
    {
        delete[] new_multiple;
    }
}

void Clipboard::convert_clipboard_target(std::shared_ptr<IncrConversion> rdata)
{
    KLOG_PROFILE("Target: %lu.", rdata->target);
    if (rdata->target == XA_TARGETS)
    {
        this->convert_type_targets(rdata);
    }
    else
    {
        this->convert_type_without_targets(rdata);
    }
}

void Clipboard::convert_type_targets(std::shared_ptr<IncrConversion> rdata)
{
    std::vector<Atom> vec_targets = this->clipboard_data_->get_targets();
    int n_targets = vec_targets.size() + 2;
    Atom* targets = new Atom[n_targets];
    if (targets == nullptr)
    {
        KLOG_ERROR("Malloc memory failed.");
        return;
    }

    n_targets = 0;
    targets[n_targets++] = XA_TARGETS;
    targets[n_targets++] = XA_MULTIPLE;

    for (auto iter : vec_targets)
    {
        targets[n_targets++] = iter;
    }

    XChangeProperty(this->display_, rdata->requestor,
                    rdata->property, XA_ATOM,
                    32, PropModeReplace,
                    (unsigned char*)targets, n_targets);

    delete[] targets;
}

void Clipboard::convert_type_without_targets(std::shared_ptr<IncrConversion> rdata)
{
    std::shared_ptr<TargetData> tdata = this->clipboard_data_->get_target_data_by_target(rdata->target);
    RETURN_IF_TRUE(tdata == nullptr);

    if (tdata->type == XA_INCR)
    {
        // we haven't completely received this target yet.
        rdata->property = None;
        return;
    }

    int bytes_per_item = ClipboardUtils::bytes_per_item(tdata->format);
    if (bytes_per_item == 0)
    {
        return;
    }

    unsigned long nitems = tdata->length / bytes_per_item;
    rdata->data = tdata;

    if (tdata->length <= SELECTION_MAX_SIZE)
    {
        XChangeProperty(this->display_, rdata->requestor,
                        rdata->property, tdata->type,
                        tdata->format, PropModeReplace,
                        tdata->data, nitems);
    }
    else
    {
        // Start incremental transfer.
        XWindowAttributes atts;
        rdata->offset = INCR_OFFSET_START;

        GdkDisplay* gdkdisplay = gdk_display_get_default();
        gdk_x11_display_error_trap_push(gdkdisplay);

        XGetWindowAttributes(this->display_, rdata->requestor, &atts);
        ClipboardUtils::change_window_filter(rdata->requestor,
                                             FILTER_CHANGE_ADD,
                                             this->filter_func_,
                                             this->user_data_);

        XSelectInput(this->display_,
                     rdata->requestor,
                     atts.your_event_mask | PropertyChangeMask);

        XChangeProperty(this->display_, rdata->requestor,
                        rdata->property, XA_INCR,
                        32, PropModeReplace,
                        (unsigned char*)&nitems, 1);
        XSync(this->display_, False);

        gdk_x11_display_error_trap_pop_ignored(gdkdisplay);
    }
}

void Clipboard::collect_incremental(std::shared_ptr<IncrConversion> rdata)
{
    if (rdata->offset != INCR_OFFSET_NONE)
    {
        KLOG_DEBUG("target: %lu", rdata->target);
        this->conversions_.push_back(rdata);
    }
}

}  // namespace Kiran