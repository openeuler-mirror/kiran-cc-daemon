/**
 * @file          /kiran-cc-daemon/plugins/audio/pulse/pulse-sink-input.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#include "plugins/audio/pulse/pulse-sink-input.h"
#include "plugins/audio/pulse/pulse-context.h"

namespace Kiran
{
PulseSinkInput::PulseSinkInput(std::shared_ptr<PulseContext> context,
                               const pa_sink_input_info *sink_input_info) : PulseStream(PulseStreamInfo(sink_input_info)),
                                                                            context_(context)
{
}

void PulseSinkInput::update(const pa_sink_input_info *sink_input_info)
{
    RETURN_IF_FALSE(sink_input_info != NULL);
    this->PulseStream::update(PulseStreamInfo(sink_input_info));
}

bool PulseSinkInput::set_mute(int32_t mute)
{
    return this->context_->set_sink_input_mute(this->get_index(), mute);
}

bool PulseSinkInput::set_cvolume(const pa_cvolume &cvolume)
{
    return this->context_->set_sink_input_volume(this->get_index(), &cvolume);
}

}  // namespace Kiran