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

#pragma once

#include <pulse/introspect.h>
#include "lib/base/base.h"

namespace Kiran
{
class PulseCardProfile
{
public:
    PulseCardProfile(const pa_card_profile_info2 *card_profile_info);
    virtual ~PulseCardProfile(){};

    const std::string &get_name() const { return this->name_; };
    const std::string &get_description() const { return this->description_; };
    uint32_t get_sink_count() const { return this->n_sinks_; };
    uint32_t get_source_count() const { return this->n_sources_; };
    uint32_t get_priority() const { return this->priority_; };

private:
    // card profile名字
    std::string name_;
    // card profile描述
    std::string description_;
    // sink数量
    uint32_t n_sinks_;
    // source数量
    uint32_t n_sources_;
    // 优先级越高，表示越适合作为默认profile
    uint32_t priority_;
};

using PulseCardProfileVec = std::vector<std::shared_ptr<PulseCardProfile>>;
}  // namespace Kiran