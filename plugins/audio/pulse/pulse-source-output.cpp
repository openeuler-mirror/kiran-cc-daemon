/**
 * @file          /kiran-cc-daemon/plugins/audio/pulse/pulse-source-output.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#include "plugins/audio/pulse/pulse-source-output.h"
#include "plugins/audio/pulse/pulse-context.h"

namespace Kiran
{
PulseSourceOutput::PulseSourceOutput(std::shared_ptr<PulseContext> context,
                                     const pa_source_output_info *source_output_info) : PulseStream(PulseStreamInfo(source_output_info)),
                                                                                        context_(context)
{
}

void PulseSourceOutput::update(const pa_source_output_info *source_output_info)
{
    RETURN_IF_FALSE(source_output_info != NULL);
    this->PulseStream::update(PulseStreamInfo(source_output_info));
}

bool PulseSourceOutput::set_mute(int32_t mute)
{
    return this->context_->set_source_output_mute(this->get_index(), mute);
}

bool PulseSourceOutput::set_cvolume(const pa_cvolume &cvolume)
{
    return this->context_->set_source_output_volume(this->get_index(), &cvolume);
}

}  // namespace Kiran