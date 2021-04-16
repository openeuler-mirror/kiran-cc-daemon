/**
 * @file          /kiran-cc-daemon/plugins/audio/pulse/pulse-card-profile.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
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