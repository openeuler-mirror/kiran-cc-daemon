/**
 * @file          /kiran-cc-daemon/plugins/audio/pulse/pulse-sink.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#include "plugins/audio/pulse/pulse-sink.h"
#include "plugins/audio/pulse/pulse-context.h"
#include "plugins/audio/pulse/pulse-port.h"

namespace Kiran
{
PulseSink::PulseSink(std::shared_ptr<PulseContext> context,
                     const pa_sink_info *sink_info) : PulseDevice(PulseDeviceInfo(sink_info)),
                                                      context_(context)
{
    if (sink_info->flags & PA_SINK_DECIBEL_VOLUME)
    {
        this->flags_ = PulseNodeState(this->flags_ | PulseNodeState::PULSE_NODE_STATE_HAS_DECIBEL);
    }
    else
    {
        this->flags_ = PulseNodeState(this->flags_ & ~PulseNodeState::PULSE_NODE_STATE_HAS_DECIBEL);
    }
}

void PulseSink::update(const pa_sink_info *sink_info)
{
    RETURN_IF_FALSE(sink_info != NULL);
    this->PulseDevice::update(PulseDeviceInfo(sink_info));
}

bool PulseSink::set_active_port(const std::string &port_name)
{
    // 避免死循环
    RETURN_VAL_IF_TRUE(port_name == this->get_active_port(), true);

    auto port = this->get_port(port_name);
    RETURN_VAL_IF_FALSE(port, false);
    return this->context_->set_sink_active_port(this->get_index(), port_name);
}

bool PulseSink::set_mute(int32_t mute)
{
    return this->context_->set_sink_mute(this->get_index(), mute);
}

bool PulseSink::set_cvolume(const pa_cvolume &cvolume)
{
    return this->context_->set_sink_volume(this->get_index(), &cvolume);
}

}  // namespace Kiran