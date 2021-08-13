/**
 * @Copyright (C) 2020 ~ 2021 KylinSec Co., Ltd. 
 *
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; If not, see <http: //www.gnu.org/licenses/>. 
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
            KLOG_WARNING("The port %s is already exist.", card_port->get_name().c_str());
        }
    }

    for (uint32_t i = 0; i < card_info->n_profiles; ++i)
    {
        auto card_profile = std::make_shared<PulseCardProfile>(card_info->profiles2[i]);
        auto iter = this->card_profiles_.emplace(card_profile->get_name(), card_profile);
        if (!iter.second)
        {
            KLOG_WARNING("The profile %s is already exist.", card_profile->get_name().c_str());
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