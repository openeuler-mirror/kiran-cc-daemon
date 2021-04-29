/**
 * @file          /kiran-cc-daemon/plugins/audio/pulse/pulse-stream.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#include "plugins/audio/pulse/pulse-stream.h"

namespace Kiran
{
PulseStreamInfo::PulseStreamInfo(const pa_sink_input_info *sink_input_info) : index(sink_input_info->index),
                                                                              name(POINTER_TO_STRING(sink_input_info->name)),
                                                                              channel_map(sink_input_info->channel_map),
                                                                              cvolume(sink_input_info->volume),
                                                                              mute(sink_input_info->mute),
                                                                              has_volume(sink_input_info->has_volume),
                                                                              volume_writable(sink_input_info->volume_writable)
{
    if (sink_input_info->proplist)
    {
        auto app_name = pa_proplist_gets(sink_input_info->proplist, PA_PROP_APPLICATION_NAME);
        this->application_name = POINTER_TO_STRING(app_name);

        auto app_icon_name = pa_proplist_gets(sink_input_info->proplist, PA_PROP_APPLICATION_ICON_NAME);
        this->icon_name = POINTER_TO_STRING(app_icon_name);
    }
}

PulseStreamInfo::PulseStreamInfo(const pa_source_output_info *source_output_info) : index(source_output_info->index),
                                                                                    name(POINTER_TO_STRING(source_output_info->name)),
                                                                                    channel_map(source_output_info->channel_map),
                                                                                    cvolume(source_output_info->volume),
                                                                                    mute(source_output_info->mute),
                                                                                    has_volume(source_output_info->has_volume),
                                                                                    volume_writable(source_output_info->volume_writable)
{
    if (source_output_info->proplist)
    {
        auto app_name = pa_proplist_gets(source_output_info->proplist, PA_PROP_APPLICATION_NAME);
        this->application_name = POINTER_TO_STRING(app_name);

        auto app_icon_name = pa_proplist_gets(source_output_info->proplist, PA_PROP_APPLICATION_ICON_NAME);
        this->icon_name = POINTER_TO_STRING(app_icon_name);
    }
}

PulseStream::PulseStream(const PulseStreamInfo &stream_info) : PulseNode(stream_info.index,
                                                                         stream_info.name,
                                                                         stream_info.channel_map,
                                                                         stream_info.cvolume,
                                                                         stream_info.mute,
                                                                         0),
                                                               application_name_(stream_info.application_name),
                                                               icon_name_(stream_info.icon_name)
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
    if (this->icon_name_ != stream_info.icon_name)
    {
        this->icon_name_ = stream_info.icon_name;
        this->icon_name_changed_.emit(this->icon_name_);
    }

    this->PulseNode::update(stream_info.channel_map, stream_info.cvolume, stream_info.mute, 0);
}
}  // namespace Kiran