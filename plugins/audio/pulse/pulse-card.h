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
#include "plugins/audio/pulse/pulse-card-port.h"
#include "plugins/audio/pulse/pulse-card-profile.h"
#include "plugins/audio/pulse/pulse-stream.h"

namespace Kiran
{
class PulseCard
{
public:
    PulseCard(const pa_card_info *card_info);
    virtual ~PulseCard();

    void update(const pa_card_info *card_info);

    uint32_t get_index() const { return this->index_; };
    const std::string &get_name() const { return this->name_; };

    std::shared_ptr<PulseCardProfile> get_profile(const std::string &name) const { return MapHelper::get_value(this->card_profiles_, name); };
    std::shared_ptr<PulseCardProfile> get_active_profile() const { return this->get_profile(this->active_profile_name_); };
    std::shared_ptr<PulseCardPort> get_card_port(const std::string &name) const { return MapHelper::get_value(this->card_ports_, name); };

    sigc::signal<void, std::shared_ptr<PulseCardProfile>> &signal_active_profile_changed() { return this->active_profile_changed_; };

private:
    void load(const pa_card_info *card_info);

private:
    uint32_t index_;
    std::string name_;

    std::map<std::string, std::shared_ptr<PulseCardProfile>> card_profiles_;
    std::string active_profile_name_;
    std::map<std::string, std::shared_ptr<PulseCardPort>> card_ports_;
    std::map<std::string, std::shared_ptr<PulseNode>> streams_;

    sigc::signal<void, std::shared_ptr<PulseCardProfile>> active_profile_changed_;
};

using PulseCardVec = std::vector<std::shared_ptr<PulseCard>>;
}  // namespace  Kiran
