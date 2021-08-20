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