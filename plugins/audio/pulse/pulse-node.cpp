/**
 * @file          /kiran-cc-daemon/plugins/audio/pulse/pulse-node.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#include "plugins/audio/pulse/pulse-stream.h"

namespace Kiran
{
PulseNode::PulseNode(uint32_t index,
                     const std::string &name,
                     const pa_channel_map &channel_map,
                     const pa_cvolume &cvolume,
                     int32_t mute,
                     pa_volume_t base_volume) : flags_(AudioNodeState::AUDIO_NODE_STATE_NONE),
                                                index_(index),
                                                name_(name),
                                                channel_map_(channel_map),
                                                cvolume_(cvolume),
                                                base_volume_(base_volume),
                                                mute_(mute != 0),
                                                volume_(PA_VOLUME_MUTED),
                                                balance_(0.0),
                                                fade_(0.0)
{
    this->update_flags();
}

bool PulseNode::set_mute(bool mute)
{
    SETTINGS_PROFILE("mute: %d.", mute);

    RETURN_VAL_IF_TRUE(this->mute_ == mute, true);

    this->update_mute(mute);
    return this->set_mute(int32_t(mute));
}

bool PulseNode::set_volume(uint32_t volume)
{
    SETTINGS_PROFILE("volume: %d.", volume);

    if (!(this->flags_ & AudioNodeState::AUDIO_NODE_STATE_VOLUME_WRITABLE))
    {
        LOG_WARNING("The volume isn't writable, flags: %x.", this->flags_);
        return false;
    }

    // 这里需要跟原始音量对比，否则可能初夏死循环，因为其他模块可能在后面的信号处理中又调用set_volume
    RETURN_VAL_IF_TRUE(this->volume_ == volume, true);
    this->update_volume(volume);

    // 将一维volume转化为cvolume
    auto cvolume = this->cvolume_;
    RETURN_VAL_IF_TRUE(pa_cvolume_scale(&cvolume, (pa_volume_t)volume) == NULL, false);
    return this->update_cvolume(cvolume);
}

bool PulseNode::set_balance(float balance)
{
    SETTINGS_PROFILE("balance: %f.", balance);

    if (!(this->flags_ & AudioNodeState::AUDIO_NODE_STATE_CAN_BALANCE))
    {
        LOG_WARNING("The balance is unsupported, flags: %x.", this->flags_);
        return false;
    }

    RETURN_VAL_IF_TRUE(std::fabs(this->balance_ - balance) < EPS, true);
    this->update_banlance(balance);

    auto cvolume = this->cvolume_;
    RETURN_VAL_IF_FALSE(pa_cvolume_set_balance(&cvolume, &this->channel_map_, balance) != NULL, false);
    return this->update_cvolume(cvolume);
}

bool PulseNode::set_fade(float fade)
{
    SETTINGS_PROFILE("fade: %f.", fade);

    if (!(this->flags_ & AudioNodeState::AUDIO_NODE_STATE_CAN_FADE))
    {
        LOG_WARNING("The fade is unsupported, flags: %x.", this->flags_);
        return false;
    }

    RETURN_VAL_IF_TRUE(std::fabs(this->fade_ - fade) < EPS, true);
    this->update_fade(fade);

    auto cvolume = this->cvolume_;
    RETURN_VAL_IF_FALSE(pa_cvolume_set_fade(&cvolume, &this->channel_map_, fade) != NULL, false);
    return this->update_cvolume(cvolume);
}

uint32_t PulseNode::get_max_volume()
{
    /*
     * From PulseAudio wiki:
     * For all volumes that are > PA_VOLUME_NORM (i.e. beyond the maximum volume
     * setting of the hw) PA will do digital amplification. This works only for
     * devices that have PA_SINK_DECIBEL_VOLUME/PA_SOURCE_DECIBEL_VOLUME set. For
     * devices that lack this flag do not extend the volume slider like this, it
     * will not have any effect.
     */
    if (this->flags_ & AudioNodeState::AUDIO_NODE_STATE_HAS_DECIBEL)
    {
        return uint32_t(PA_VOLUME_UI_MAX);
    }
    else
    {
        return uint32_t(PA_VOLUME_NORM);
    }
}

uint32_t PulseNode::get_base_volume()
{
    RETURN_VAL_IF_TRUE(this->base_volume_ > 0, this->base_volume_);
    return uint32_t(PA_VOLUME_NORM);
}

void PulseNode::update(const pa_channel_map &channel_map,
                       const pa_cvolume &cvolume,
                       int32_t mute,
                       pa_volume_t base_volume)
{
    this->cvolume_ = cvolume;
    this->channel_map_ = channel_map;
    this->base_volume_ = base_volume;

    this->update_flags();

    this->update_mute(mute != 0);

    // 更新一维音量，取所有通道的最大音量
    auto volume = (uint32_t)pa_cvolume_max(&cvolume);
    this->update_volume(volume);

    // 更新一维balance
    auto balance = pa_cvolume_get_balance(&cvolume, &channel_map);
    this->update_banlance(balance);

    // 更新fade
    auto fade = pa_cvolume_get_fade(&cvolume, &channel_map);
    this->update_fade(fade);
}

bool PulseNode::set_mute(int32_t)
{
    // 不支持，需要子类实现
    return false;
}

bool PulseNode::set_cvolume(const pa_cvolume &)
{
    // 不支持，需要子类实现
    return false;
}

void PulseNode::update_flags()
{
    SETTINGS_PROFILE("");

    LOG_DEBUG("flags before updated: %x.", this->flags_);
    if (pa_channel_map_valid(&this->channel_map_))
    {
        if (pa_channel_map_can_balance(&this->channel_map_))
        {
            this->flags_ = AudioNodeState(this->flags_ | AudioNodeState::AUDIO_NODE_STATE_CAN_BALANCE);
        }
        else
        {
            this->flags_ = AudioNodeState(this->flags_ & ~AudioNodeState::AUDIO_NODE_STATE_CAN_BALANCE);
        }

        if (pa_channel_map_can_fade(&this->channel_map_))
        {
            this->flags_ = AudioNodeState(this->flags_ | AudioNodeState::AUDIO_NODE_STATE_CAN_FADE);
        }
        else
        {
            this->flags_ = AudioNodeState(this->flags_ & ~AudioNodeState::AUDIO_NODE_STATE_CAN_FADE);
        }
    }
    else
    {
        this->flags_ = AudioNodeState(this->flags_ & ~(AudioNodeState::AUDIO_NODE_STATE_CAN_BALANCE | AudioNodeState::AUDIO_NODE_STATE_CAN_BALANCE));
    }

    if (pa_cvolume_valid(&this->cvolume_))
    {
        this->flags_ = AudioNodeState(this->flags_ | (AudioNodeState::AUDIO_NODE_STATE_VOLUME_READABLE |
                                                      AudioNodeState::AUDIO_NODE_STATE_VOLUME_WRITABLE));
    }
    else
    {
        this->flags_ = AudioNodeState(this->flags_ & ~(AudioNodeState::AUDIO_NODE_STATE_VOLUME_READABLE |
                                                       AudioNodeState::AUDIO_NODE_STATE_VOLUME_WRITABLE));

        this->set_mute(true);
    }

    LOG_DEBUG("flags after updated: %x.", this->flags_);
}

void PulseNode::update_mute(bool mute)
{
    RETURN_IF_TRUE(this->mute_ == mute);
    this->mute_ = mute;
    this->node_info_changed_.emit(PulseNodeField::PULSE_NODE_FIELD_MUTE);
    return;
}

void PulseNode::update_volume(uint32_t volume)
{
    if (this->volume_ != volume)
    {
        this->volume_ = volume;
        this->node_info_changed_.emit(PulseNodeField::PULSE_NODE_FIELD_VOLUME);
    }
}

void PulseNode::update_banlance(float balance)
{
    if (this->balance_ != balance)
    {
        this->balance_ = balance;
        this->node_info_changed_.emit(PulseNodeField::PULSE_NODE_FIELD_BALANCE);
    }
}

void PulseNode::update_fade(float fade)
{
    if (this->fade_ != fade)
    {
        this->fade_ = fade;
        this->node_info_changed_.emit(PulseNodeField::PULSE_NODE_FIELD_FADE);
    }
}

bool PulseNode::update_cvolume(const pa_cvolume &cvolume)
{
    RETURN_VAL_IF_FALSE(pa_cvolume_valid(&cvolume) != 0, false);
    RETURN_VAL_IF_TRUE(pa_cvolume_equal(&this->cvolume_, &cvolume) != 0, true);

    this->cvolume_ = cvolume;
    this->set_cvolume(cvolume);
    return true;
}

}  // namespace Kiran