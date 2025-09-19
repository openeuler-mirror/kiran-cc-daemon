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

#include "pulse-node.h"
#include "lib/base/base.h"
#include "pulse-stream.h"

namespace Kiran
{
PulseNodeInfo::PulseNodeInfo() : index(0),
                                 mute(0),
                                 baseVolume(0),
                                 proplist(NULL)
{
    memset(&channelMap, 0, sizeof(channelMap));
    memset(&cvolume, 0, sizeof(cvolume));
}

PulseNodeInfo::PulseNodeInfo(const PulseNodeInfo &other)
{
    this->index = other.index;
    this->name = other.name;
    this->channelMap = other.channelMap;
    this->cvolume = other.cvolume;
    this->mute = other.mute;
    this->baseVolume = other.baseVolume;
    this->proplist = other.proplist;
}

PulseNodeInfo::PulseNodeInfo(const pa_sink_info *sinkInfo)
{
    this->index = sinkInfo->index;
    this->name = POINTER_TO_STRING(sinkInfo->name);
    this->channelMap = sinkInfo->channel_map;
    this->cvolume = sinkInfo->volume;
    this->mute = sinkInfo->mute;
    this->baseVolume = sinkInfo->base_volume;
    this->proplist = sinkInfo->proplist;
}

PulseNodeInfo::PulseNodeInfo(const pa_source_info *sourceInfo)
{
    this->index = sourceInfo->index;
    this->name = POINTER_TO_STRING(sourceInfo->name);
    this->channelMap = sourceInfo->channel_map;
    this->cvolume = sourceInfo->volume;
    this->mute = sourceInfo->mute;
    this->baseVolume = sourceInfo->base_volume;
    this->proplist = sourceInfo->proplist;
}

PulseNodeInfo::PulseNodeInfo(const pa_sink_input_info *sinkInputInfo)
{
    this->index = sinkInputInfo->index;
    this->name = POINTER_TO_STRING(sinkInputInfo->name);
    this->channelMap = sinkInputInfo->channel_map;
    this->cvolume = sinkInputInfo->volume;
    this->mute = sinkInputInfo->mute;
    this->baseVolume = 0;
    this->proplist = sinkInputInfo->proplist;
}

PulseNodeInfo::PulseNodeInfo(const pa_source_output_info *sourceOutputInfo)
{
    this->index = sourceOutputInfo->index;
    this->name = POINTER_TO_STRING(sourceOutputInfo->name);
    this->channelMap = sourceOutputInfo->channel_map;
    this->cvolume = sourceOutputInfo->volume;
    this->mute = sourceOutputInfo->mute;
    this->baseVolume = 0;
    this->proplist = sourceOutputInfo->proplist;
}

PulseNode::PulseNode(const PulseNodeInfo &nodeInfo) : m_flags(AudioNodeState::AUDIO_NODE_STATE_NONE),
                                                      m_index(nodeInfo.index),
                                                      m_name(nodeInfo.name),
                                                      m_channelMap(nodeInfo.channelMap),
                                                      m_cvolume(nodeInfo.cvolume),
                                                      m_baseVolume(nodeInfo.baseVolume),
                                                      m_mute(nodeInfo.mute != 0),
                                                      m_volume(PA_VOLUME_MUTED),
                                                      m_balance(0.0),
                                                      m_fade(0.0)
{
    // 解析所有属性
    void *state = NULL;
    auto key = pa_proplist_iterate(nodeInfo.proplist, &state);
    for (; key != NULL; key = pa_proplist_iterate(nodeInfo.proplist, &state))
    {
        m_attrs[key] = POINTER_TO_STRING(pa_proplist_gets(nodeInfo.proplist, key));
    }

    update(nodeInfo.channelMap, nodeInfo.cvolume, nodeInfo.mute, nodeInfo.baseVolume);
}

bool PulseNode::setMute(bool mute)
{
    RETURN_VAL_IF_TRUE(m_mute == mute, true);

    updateMute(mute);
    return setMute(int32_t(mute));
}

bool PulseNode::setVolume(uint32_t volume)
{
    KLOG_INFO(audio) << "Ready to set volume to" << volume;

    if (!(m_flags & AudioNodeState::AUDIO_NODE_STATE_VOLUME_WRITABLE))
    {
        KLOG_WARNING(audio) << "The volume isn't writable, flags is" << m_flags;
        return false;
    }

    // 这里需要跟原始音量对比，否则可能出现死循环，因为其他模块可能在后面的信号处理中又调用set_volume
    RETURN_VAL_IF_TRUE(m_volume == volume, true);
    updateVolume(volume);

    // 将一维volume转化为cvolume
    auto cvolume = m_cvolume;
    RETURN_VAL_IF_TRUE(pa_cvolume_scale(&cvolume, (pa_volume_t)volume) == NULL, false);
    return updateCvolume(cvolume);
}

bool PulseNode::setBalance(float balance)
{
    KLOG_INFO(audio) << "Set balance to" << balance;

    if (!(m_flags & AudioNodeState::AUDIO_NODE_STATE_CAN_BALANCE))
    {
        KLOG_WARNING(audio) << "The balance is unsupported, flags is" << m_flags;
        return false;
    }

    RETURN_VAL_IF_TRUE(std::fabs(m_balance - balance) < EPS, true);
    updateBanlance(balance);

    auto cvolume = m_cvolume;
    RETURN_VAL_IF_FALSE(pa_cvolume_set_balance(&cvolume, &m_channelMap, balance) != NULL, false);
    return updateCvolume(cvolume);
}

bool PulseNode::setFade(float fade)
{
    if (!(m_flags & AudioNodeState::AUDIO_NODE_STATE_CAN_FADE))
    {
        KLOG_WARNING(audio) << "The fade is unsupported, flags is" << m_flags;
        return false;
    }

    RETURN_VAL_IF_TRUE(std::fabs(m_fade - fade) < EPS, true);
    updateFade(fade);

    auto cvolume = m_cvolume;
    RETURN_VAL_IF_FALSE(pa_cvolume_set_fade(&cvolume, &m_channelMap, fade) != NULL, false);
    return updateCvolume(cvolume);
}

uint32_t PulseNode::getMaxVolume()
{
    /*
     * From PulseAudio wiki:
     * For all volumes that are > PA_VOLUME_NORM (i.e. beyond the maximum volume
     * setting of the hw) PA will do digital amplification. This works only for
     * devices that have PA_SINK_DECIBEL_VOLUME/PA_SOURCE_DECIBEL_VOLUME set. For
     * devices that lack this flag do not extend the volume slider like this, it
     * will not have any effect.
     */
    if (m_flags & AudioNodeState::AUDIO_NODE_STATE_HAS_DECIBEL)
    {
        return uint32_t(PA_VOLUME_UI_MAX);
    }
    else
    {
        return uint32_t(PA_VOLUME_NORM);
    }
}

uint32_t PulseNode::getBaseVolume()
{
    RETURN_VAL_IF_TRUE(m_baseVolume > 0, m_baseVolume);
    return uint32_t(PA_VOLUME_NORM);
}

void PulseNode::update(const pa_channel_map &channel_map,
                       const pa_cvolume &cvolume,
                       int32_t mute,
                       pa_volume_t base_volume)
{
    m_cvolume = cvolume;
    m_channelMap = channel_map;
    m_baseVolume = base_volume;

    updateFlags();

    updateMute(mute != 0);

    // 更新一维音量，取所有通道的最大音量
    auto volume = (uint32_t)pa_cvolume_max(&cvolume);
    updateVolume(volume);

    // 更新一维balance
    auto balance = pa_cvolume_get_balance(&cvolume, &channel_map);
    updateBanlance(balance);

    // 更新fade
    auto fade = pa_cvolume_get_fade(&cvolume, &channel_map);
    updateFade(fade);
}

bool PulseNode::setMute(int32_t)
{
    // 不支持，需要子类实现
    return false;
}

bool PulseNode::setCvolume(const pa_cvolume &)
{
    // 不支持，需要子类实现
    return false;
}

void PulseNode::updateFlags()
{
    KLOG_DEBUG(audio) << "Flags before updated:" << m_flags;

    if (pa_channel_map_valid(&m_channelMap))
    {
        if (pa_channel_map_can_balance(&m_channelMap))
        {
            m_flags = AudioNodeState(m_flags | AudioNodeState::AUDIO_NODE_STATE_CAN_BALANCE);
        }
        else
        {
            m_flags = AudioNodeState(m_flags & ~AudioNodeState::AUDIO_NODE_STATE_CAN_BALANCE);
        }

        if (pa_channel_map_can_fade(&m_channelMap))
        {
            m_flags = AudioNodeState(m_flags | AudioNodeState::AUDIO_NODE_STATE_CAN_FADE);
        }
        else
        {
            m_flags = AudioNodeState(m_flags & ~AudioNodeState::AUDIO_NODE_STATE_CAN_FADE);
        }
    }
    else
    {
        m_flags = AudioNodeState(m_flags & ~(AudioNodeState::AUDIO_NODE_STATE_CAN_BALANCE | AudioNodeState::AUDIO_NODE_STATE_CAN_BALANCE));
    }

    if (pa_cvolume_valid(&m_cvolume))
    {
        m_flags = AudioNodeState(m_flags | (AudioNodeState::AUDIO_NODE_STATE_VOLUME_READABLE |
                                            AudioNodeState::AUDIO_NODE_STATE_VOLUME_WRITABLE));
    }
    else
    {
        m_flags = AudioNodeState(m_flags & ~(AudioNodeState::AUDIO_NODE_STATE_VOLUME_READABLE |
                                             AudioNodeState::AUDIO_NODE_STATE_VOLUME_WRITABLE));
    }

    KLOG_DEBUG(audio) << "Flags after updated:" << m_flags;
}

void PulseNode::updateMute(bool mute)
{
    RETURN_IF_TRUE(m_mute == mute);
    m_mute = mute;
    Q_EMIT nodeInfoChanged(PulseNodeField::PULSE_NODE_FIELD_MUTE);
}

void PulseNode::updateVolume(uint32_t volume)
{
    if (m_volume != volume)
    {
        m_volume = volume;
        Q_EMIT nodeInfoChanged(PulseNodeField::PULSE_NODE_FIELD_VOLUME);
    }
}

void PulseNode::updateBanlance(float balance)
{
    if (m_balance != balance)
    {
        m_balance = balance;
        Q_EMIT nodeInfoChanged(PulseNodeField::PULSE_NODE_FIELD_BALANCE);
    }
}

void PulseNode::updateFade(float fade)
{
    if (m_fade != fade)
    {
        m_fade = fade;
        Q_EMIT nodeInfoChanged(PulseNodeField::PULSE_NODE_FIELD_FADE);
    }
}

bool PulseNode::updateCvolume(const pa_cvolume &cvolume)
{
    RETURN_VAL_IF_FALSE(pa_cvolume_valid(&cvolume) != 0, false);
    RETURN_VAL_IF_TRUE(pa_cvolume_equal(&m_cvolume, &cvolume) != 0, true);

    m_cvolume = cvolume;
    setCvolume(cvolume);
    return true;
}

}  // namespace Kiran