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

#include "pulse-stream.h"
#include "lib/base/base.h"

namespace Kiran
{
PulseStreamInfo::PulseStreamInfo(const pa_sink_input_info *sinkInputInfo) : PulseNodeInfo(PulseNodeInfo{.index = sinkInputInfo->index,
                                                                                                        .name = POINTER_TO_STRING(sinkInputInfo->name),
                                                                                                        .channelMap = sinkInputInfo->channel_map,
                                                                                                        .cvolume = sinkInputInfo->volume,
                                                                                                        .mute = sinkInputInfo->mute,
                                                                                                        .baseVolume = 0,
                                                                                                        .proplist = sinkInputInfo->proplist}),
                                                                            hasVolume(sinkInputInfo->has_volume),
                                                                            volumeWritable(sinkInputInfo->volume_writable)
{
}

PulseStreamInfo::PulseStreamInfo(const pa_source_output_info *sourceOutputInfo) : PulseNodeInfo(PulseNodeInfo{.index = sourceOutputInfo->index,
                                                                                                              .name = POINTER_TO_STRING(sourceOutputInfo->name),
                                                                                                              .channelMap = sourceOutputInfo->channel_map,
                                                                                                              .cvolume = sourceOutputInfo->volume,
                                                                                                              .mute = sourceOutputInfo->mute,
                                                                                                              .baseVolume = 0,
                                                                                                              .proplist = sourceOutputInfo->proplist}),
                                                                                  hasVolume(sourceOutputInfo->has_volume),
                                                                                  volumeWritable(sourceOutputInfo->volume_writable)
{
}

PulseStream::PulseStream(const PulseStreamInfo &streamInfo) : PulseNode(streamInfo)
{
    // has_volume如果为false，说明volume字段是未定义的，这个在基类中已经通过pa_cvolume_valid做过判断了，这里再校验一次
    if (!streamInfo.hasVolume)
    {
        m_flags = AudioNodeState(m_flags & ~(AudioNodeState::AUDIO_NODE_STATE_VOLUME_READABLE |
                                           AudioNodeState::AUDIO_NODE_STATE_VOLUME_WRITABLE));
    }

    /* freedeskotp wiki: For playback devices it might be advisable to extend the scale beyond PA_VOLUME_NORM as well,
    because often enough digital amplification is useful on limited hardware.*/
    if (streamInfo.hasVolume)
    {
        m_flags = AudioNodeState(m_flags | AudioNodeState::AUDIO_NODE_STATE_HAS_DECIBEL);
    }

    if (!streamInfo.volumeWritable)
    {
        m_flags = AudioNodeState(m_flags & ~(AudioNodeState::AUDIO_NODE_STATE_VOLUME_WRITABLE));
    }
}

void PulseStream::update(const PulseStreamInfo &streamInfo)
{
    PulseNode::update(streamInfo.channelMap, streamInfo.cvolume, streamInfo.mute, 0);
}
}  // namespace Kiran