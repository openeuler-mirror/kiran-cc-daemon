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

#include "plugins/audio/pulse/pulse-stream.h"

namespace Kiran
{
PulseStreamInfo::PulseStreamInfo(const pa_sink_input_info *sink_input_info) : PulseNodeInfo(PulseNodeInfo{.index = sink_input_info->index,
                                                                                                          .name = POINTER_TO_STRING(sink_input_info->name),
                                                                                                          .channel_map = sink_input_info->channel_map,
                                                                                                          .cvolume = sink_input_info->volume,
                                                                                                          .mute = sink_input_info->mute,
                                                                                                          .base_volume = 0,
                                                                                                          .proplist = sink_input_info->proplist}),
                                                                              has_volume(sink_input_info->has_volume),
                                                                              volume_writable(sink_input_info->volume_writable)
{
}

PulseStreamInfo::PulseStreamInfo(const pa_source_output_info *source_output_info) : PulseNodeInfo(PulseNodeInfo{.index = source_output_info->index,
                                                                                                                .name = POINTER_TO_STRING(source_output_info->name),
                                                                                                                .channel_map = source_output_info->channel_map,
                                                                                                                .cvolume = source_output_info->volume,
                                                                                                                .mute = source_output_info->mute,
                                                                                                                .base_volume = 0,
                                                                                                                .proplist = source_output_info->proplist}),
                                                                                    has_volume(source_output_info->has_volume),
                                                                                    volume_writable(source_output_info->volume_writable)
{
}

PulseStream::PulseStream(const PulseStreamInfo &stream_info) : PulseNode(stream_info)
{
    // has_volume如果为false，说明volume字段是未定义的，这个在基类中已经通过pa_cvolume_valid做过判断了，这里再校验一次
    if (!stream_info.has_volume)
    {
        this->flags_ = AudioNodeState(this->flags_ & ~(AudioNodeState::AUDIO_NODE_STATE_VOLUME_READABLE |
                                                       AudioNodeState::AUDIO_NODE_STATE_VOLUME_WRITABLE));
    }

    /* freedeskotp wiki: For playback devices it might be advisable to extend the scale beyond PA_VOLUME_NORM as well,
    because often enough digital amplification is useful on limited hardware.*/
    if (stream_info.has_volume)
    {
        this->flags_ = AudioNodeState(this->flags_ | AudioNodeState::AUDIO_NODE_STATE_HAS_DECIBEL);
    }

    if (!stream_info.volume_writable)
    {
        this->flags_ = AudioNodeState(this->flags_ & ~(AudioNodeState::AUDIO_NODE_STATE_VOLUME_WRITABLE));
    }
}

void PulseStream::update(const PulseStreamInfo &stream_info)
{
    this->PulseNode::update(stream_info.channel_map, stream_info.cvolume, stream_info.mute, 0);
}
}  // namespace Kiran