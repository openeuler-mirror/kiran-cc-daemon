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

double AudioStream::volume_get()
{
    auto volume = this->stream_->get_volume();
    return AudioUtils::volume_absolute2range(volume, this->stream_->get_min_volume(), this->stream_->get_max_volume());
}

void AudioStream::SetVolume(double volume, MethodInvocation &invocation)
{
    if (volume < 0 || volume > 1.0 + EPS)
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_AUDIO_STREAM_VOLUME_RANGE_INVLAID);
    }

    if (!this->volume_set(volume))
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
    if (!this->mute_set(mute))
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

bool AudioStream::volume_setHandler(double value)
{
    auto volume_absolute = AudioUtils::volume_range2absolute(value,
                                                             this->stream_->get_min_volume(),
                                                             this->stream_->get_max_volume());

    return this->stream_->set_volume(volume_absolute);
}

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
    // 这里的主要目的是为了触发dbus属性信号，小心使用，避免出现死循环
    switch (field)
    {
    case PulseNodeField::PULSE_NODE_FIELD_MUTE:
        this->mute_set(this->mute_get());
        break;
    case PulseNodeField::PULSE_NODE_FIELD_VOLUME:
        this->volume_set(this->volume_get());
        break;
    default:
        break;
    }
}

}  // namespace Kiran