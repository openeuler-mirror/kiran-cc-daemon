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

#include "plugins/clipboard/clipboard-data.h"

namespace Kiran
{
ClipboardData::ClipboardData()
{
}

ClipboardData::~ClipboardData()
{
}

void ClipboardData::init()
{
}

void ClipboardData::deinit()
{
    this->clear_contents();
}

void ClipboardData::clear_contents()
{
    this->contents_.clear();
}

bool ClipboardData::is_contents_empty()
{
    return this->contents_.empty();
}

bool ClipboardData::is_exist_type(Atom type)
{
    for (auto iter : this->contents_)
    {
        if (iter.second->type == type)
        {
            return true;
        }
    }

    return false;
}

void ClipboardData::add_target_data(Atom target)
{
    std::shared_ptr<TargetData> target_ptr = std::make_shared<TargetData>();
    target_ptr->target = target;
    this->contents_.emplace(target, target_ptr);
}

std::vector<Atom> ClipboardData::get_targets()
{
    std::vector<Atom> vec_targets;
    for (auto iter : this->contents_)
    {
        vec_targets.push_back(iter.second->target);
    }

    return vec_targets;
}

std::shared_ptr<TargetData>
ClipboardData::get_target_data_by_target(Atom target)
{
    auto iter = this->contents_.find(target);
    if (iter != this->contents_.end())
    {
        return iter->second;
    }
    return nullptr;
}

void ClipboardData::save_targets_data(Display *display, Window window)
{
    std::vector<Atom> targets = this->get_targets();
    for (auto iter : targets)
    {
        this->save_target_data(display, window, iter);
    }
}

void ClipboardData::save_target_data(Display *display, Window window, Atom target)
{
    auto iter = this->contents_.find(target);
    if (iter == this->contents_.end())
    {
        return;
    }

    std::shared_ptr<TargetData> tdata = iter->second;
    WindowPropertyGroup prop_group;
    bool ret = ClipboardUtils::get_window_property_group(display,
                                                         window,
                                                         tdata->target,
                                                         True, AnyPropertyType,
                                                         &prop_group);
    RETURN_IF_FALSE(ret);

    if (prop_group.type == None)
    {
        this->contents_.erase(iter);
    }
    else if (prop_group.type == XA_INCR)
    {
        tdata->type = prop_group.type;
        tdata->length = 0;
    }
    else
    {
        tdata->type = prop_group.type;
        tdata->format = prop_group.format;

        unsigned long length = prop_group.nitems * ClipboardUtils::bytes_per_item(prop_group.format);
        tdata->data = new unsigned char[length + 1];
        if (tdata->data == nullptr)
        {
            KLOG_ERROR_CLIPBOARD("Malloc memory size: %lu failed", length);
            return;
        }

        tdata->length = length;
        memcpy(tdata->data, prop_group.data, tdata->length);

        KLOG_DEBUG_CLIPBOARD("Target is %lu, format is %d and the length is %lu.",
                             tdata->target, tdata->format, tdata->length);
    }
}

void ClipboardData::save_incremental_target_data(std::shared_ptr<TargetData> tdata,
                                                 const WindowPropertyGroup &prop_group)
{
    unsigned long length = prop_group.nitems * ClipboardUtils::bytes_per_item(prop_group.format);

    if (length == 0)
    {
        tdata->type = prop_group.type;
        tdata->format = prop_group.format;
    }
    else
    {
        if (!tdata->data)
        {
            tdata->data = new unsigned char[length + 1];
            if (tdata->data == nullptr)
            {
                KLOG_ERROR_CLIPBOARD("Malloc memory size: %lu failed.", length);
                return;
            }

            tdata->length = length;
            memcpy(tdata->data, prop_group.data, tdata->length);
        }
        else
        {
            unsigned char *tmp_data = new unsigned char[tdata->length + length + 1];
            if (tmp_data == nullptr)
            {
                KLOG_ERROR_CLIPBOARD("Malloc memory size: %lu failed.", tdata->length + length);
                return;
            }

            memcpy(tmp_data, tdata->data, tdata->length);
            memcpy(tmp_data + tdata->length, prop_group.data, length);

            delete[] tdata->data;

            tdata->length += length;
            tdata->data = tmp_data;
        }
    }
}

}  // namespace Kiran