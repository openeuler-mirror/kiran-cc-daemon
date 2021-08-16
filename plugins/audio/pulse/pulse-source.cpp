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

#include "plugins/audio/pulse/pulse-source.h"
#include "plugins/audio/pulse/pulse-context.h"

namespace Kiran
{
PulseSource::PulseSource(std::shared_ptr<PulseContext> context,
                         const pa_source_info *source_info) : PulseDevice(PulseDeviceInfo(source_info)),
                                                              context_(context)
{
    if (source_info->flags & PA_SOURCE_DECIBEL_VOLUME)
    {
        this->flags_ = AudioNodeState(this->flags_ | AudioNodeState::AUDIO_NODE_STATE_HAS_DECIBEL);
    }
    else
    {
        this->flags_ = AudioNodeState(this->flags_ & ~AudioNodeState::AUDIO_NODE_STATE_HAS_DECIBEL);
    }
}

void PulseSource::update(const pa_source_info *source_info)
{
    RETURN_IF_FALSE(source_info != NULL);

    this->PulseDevice::update(PulseDeviceInfo(source_info));
}

bool PulseSource::set_active_port(const std::string &port_name)
{
    // 避免死循环
    RETURN_VAL_IF_TRUE(port_name == this->get_active_port(), true);

    auto port = this->get_port(port_name);
    RETURN_VAL_IF_FALSE(port, false);
    return this->context_->set_source_active_port(this->get_index(), port_name);
}

bool PulseSource::set_mute(int32_t mute)
{
    return this->context_->set_source_mute(this->get_index(), mute);
}

bool PulseSource::set_cvolume(const pa_cvolume &cvolume)
{
    return this->context_->set_source_volume(this->get_index(), &cvolume);
}

}  // namespace Kiran