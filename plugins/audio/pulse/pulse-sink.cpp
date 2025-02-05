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

#include "pulse-sink.h"
#include "pulse-context.h"
#include "pulse-port.h"

namespace Kiran
{
PulseSink::PulseSink(QSharedPointer<PulseContext> context,
                     const pa_sink_info *sinkInfo) : PulseDevice(PulseDeviceInfo(sinkInfo)),
                                                     m_context(context)
{
    if (sinkInfo->flags & PA_SINK_DECIBEL_VOLUME)
    {
        m_flags = AudioNodeState(m_flags | AudioNodeState::AUDIO_NODE_STATE_HAS_DECIBEL);
    }
    else
    {
        m_flags = AudioNodeState(m_flags & ~AudioNodeState::AUDIO_NODE_STATE_HAS_DECIBEL);
    }
}

void PulseSink::update(const pa_sink_info *sinkInfo)
{
    RETURN_IF_FALSE(sinkInfo != NULL);
    PulseDevice::update(PulseDeviceInfo(sinkInfo));
}

bool PulseSink::setActivePort(const QString &portName)
{
    // 避免死循环
    RETURN_VAL_IF_TRUE(portName == getActivePort(), true);

    auto port = get_port(portName);
    RETURN_VAL_IF_FALSE(port, false);
    return m_context->setSinkActivePort(getIndex(), portName);
}

bool PulseSink::setMute(int32_t mute)
{
    return m_context->setSinkMute(getIndex(), mute);
}

bool PulseSink::setCvolume(const pa_cvolume &cvolume)
{
    return m_context->setSinkVolume(getIndex(), &cvolume);
}

}  // namespace Kiran