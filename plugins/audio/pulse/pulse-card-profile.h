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