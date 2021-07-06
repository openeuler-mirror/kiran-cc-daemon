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