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

#include "plugins/audio/pulse/pulse-card.h"

namespace Kiran
{
PulseCard::PulseCard(const pa_card_info *card_info) : index_(card_info->index),
                                                      name_(POINTER_TO_STRING(card_info->name))
{
    this->load(card_info);
}

void PulseCard::update(const pa_card_info *card_info)
{
    RETURN_IF_FALSE(card_info != NULL);

    std::string active_profile_name;

    // 只有active profile可能会发生变化，因此其他属性不进行更新
    if (card_info->active_profile2 != NULL)
    {
        active_profile_name = POINTER_TO_STRING(card_info->active_profile2->name);
    }

    if (this->active_profile_name_ != active_profile_name)
    {
        this->active_profile_name_ = active_profile_name;
        auto active_profile = this->get_profile(this->active_profile_name_);
        this->active_profile_changed_.emit(active_profile);
    }
}

void PulseCard::load(const pa_card_info *card_info)
{
    RETURN_IF_FALSE(card_info != NULL);

    this->card_ports_.clear();
    this->card_profiles_.clear();
    this->active_profile_name_.clear();

    for (uint32_t i = 0; i < card_info->n_ports; ++i)
    {
        auto card_port = std::make_shared<PulseCardPort>(card_info->ports[i]);
        auto iter = this->card_ports_.emplace(card_port->get_name(), card_port);
        if (!iter.second)
        {
            KLOG_WARNING_AUDIO("The port %s is already exist.", card_port->get_name().c_str());
        }
    }

    for (uint32_t i = 0; i < card_info->n_profiles; ++i)
    {
        auto card_profile = std::make_shared<PulseCardProfile>(card_info->profiles2[i]);
        auto iter = this->card_profiles_.emplace(card_profile->get_name(), card_profile);
        if (!iter.second)
        {
            KLOG_WARNING_AUDIO("The profile %s is already exist.", card_profile->get_name().c_str());
        }
    }

    if (card_info->active_profile2 != NULL)
    {
        this->active_profile_name_ = POINTER_TO_STRING(card_info->active_profile2->name);
    }
}

PulseCard::~PulseCard()
{
}
}  // namespace Kiran