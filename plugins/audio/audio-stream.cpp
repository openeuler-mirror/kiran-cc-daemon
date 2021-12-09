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

#include "plugins/audio/audio-stream.h"
#include "plugins/audio/audio-utils.h"

namespace Kiran
{
AudioStream::AudioStream(std::shared_ptr<PulseStream> stream) : stream_(stream),
                                                                object_register_id_(0)
{
    this->mute_ = this->stream_->get_mute();
    this->volume_ = AudioUtils::volume_absolute2range(this->stream_->get_volume(),
                                                      this->stream_->get_min_volume(),
                                                      this->stream_->get_max_volume());
    this->stream_->signal_node_info_changed().connect(sigc::mem_fun(this, &AudioStream::on_node_info_changed_cb));
}

AudioStream::~AudioStream()
{
    this->dbus_unregister();
}

bool AudioStream::init(const std::string &object_path_prefix)
{
    RETURN_VAL_IF_FALSE(this->stream_, false);

    this->object_path_ = fmt::format("{0}{1}", object_path_prefix, this->stream_->get_index());
    return this->dbus_register();
}

void AudioStream::SetVolume(double volume, MethodInvocation &invocation)
{
    if (volume < 0 || volume > 1.0 + EPS)
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_AUDIO_STREAM_VOLUME_RANGE_INVLAID);
    }

    auto volume_absolute = AudioUtils::volume_range2absolute(volume,
                                                             this->stream_->get_min_volume(),
                                                             this->stream_->get_max_volume());

    if (!this->stream_->set_volume(volume_absolute))
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_AUDIO_STREAM_SET_VOLUME_FAILED);
    }

    // 如果音量大于0，则取消静音
    if (volume > EPS)
    {
        this->mute_set(false);
    }

    invocation.ret();
}

void AudioStream::SetMute(bool mute, MethodInvocation &invocation)
{
    if (!this->stream_->set_mute(mute))
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_AUDIO_STREAM_SET_MUTE_FAILED);
    }

    // 如果设置了静音，则将音量也设置为0
    if (mute)
    {
        this->volume_set(0);
    }

    invocation.ret();
}

void AudioStream::GetProperty(const Glib::ustring &key, MethodInvocation &invocation)
{
    KLOG_PROFILE("key: %s.", key.c_str());

    auto value = this->stream_->get_property(key);
    invocation.ret(value);
}

#define SET_COMMON_PROPERTY(property, type)                    \
    bool AudioStream::property##_setHandler(type value)        \
    {                                                          \
        RETURN_VAL_IF_TRUE(this->property##_ == value, false); \
        this->property##_ = value;                             \
        return true;                                           \
    }

#define SET_DOUBLE_PROPERTY(property)                     \
    bool AudioStream::property##_setHandler(double value) \
    {                                                     \
        if (std::fabs(this->property##_ - value) < EPS)   \
        {                                                 \
            return false;                                 \
        }                                                 \
        this->property##_ = value;                        \
        return true;                                      \
    }

SET_COMMON_PROPERTY(mute, bool)
SET_DOUBLE_PROPERTY(volume)

bool AudioStream::dbus_register()
{
    KLOG_PROFILE("register object path: %s.", this->object_path_.c_str());

    RETURN_VAL_IF_FALSE(this->stream_, false);

    try
    {
        this->dbus_connect_ = Gio::DBus::Connection::get_sync(Gio::DBus::BUS_TYPE_SESSION);
    }
    catch (const Glib::Error &e)
    {
        KLOG_WARNING("Failed to get session bus: %s.", e.what().c_str());
        return false;
    }

    this->object_register_id_ = this->register_object(this->dbus_connect_, this->object_path_.c_str());
    return true;
}

void AudioStream::dbus_unregister()
{
    KLOG_PROFILE("unregister object path: %s.", this->object_path_.c_str());

    if (this->object_register_id_)
    {
        this->unregister_object();
        this->object_register_id_ = 0;
    }
}

void AudioStream::on_node_info_changed_cb(PulseNodeField field)
{
    switch (field)
    {
    case PulseNodeField::PULSE_NODE_FIELD_MUTE:
        this->mute_set(this->stream_->get_mute());
        break;
    case PulseNodeField::PULSE_NODE_FIELD_VOLUME:
    {
        auto volume_range = AudioUtils::volume_absolute2range(this->stream_->get_volume(),
                                                              this->stream_->get_min_volume(),
                                                              this->stream_->get_max_volume());
        this->volume_set(volume_range);
        break;
    }
    default:
        break;
    }
}

}  // namespace Kiran