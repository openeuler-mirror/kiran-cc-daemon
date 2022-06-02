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