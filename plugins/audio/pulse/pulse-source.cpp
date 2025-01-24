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

#include "pulse-source.h"
#include "pulse-context.h"

namespace Kiran
{
PulseSource::PulseSource(QSharedPointer<PulseContext> context,
                         const pa_source_info *sourceInfo) : PulseDevice(PulseDeviceInfo(sourceInfo)),
                                                             m_context(context)
{
    if (sourceInfo->flags & PA_SOURCE_DECIBEL_VOLUME)
    {
        m_flags = AudioNodeState(m_flags | AudioNodeState::AUDIO_NODE_STATE_HAS_DECIBEL);
    }
    else
    {
        m_flags = AudioNodeState(m_flags & ~AudioNodeState::AUDIO_NODE_STATE_HAS_DECIBEL);
    }
}

void PulseSource::update(const pa_source_info *source_info)
{
    RETURN_IF_FALSE(source_info != NULL);

    PulseDevice::update(PulseDeviceInfo(source_info));
}

bool PulseSource::setActivePort(const QString &portName)
{
    // 避免死循环
    RETURN_VAL_IF_TRUE(portName == getActivePort(), true);

    auto port = get_port(portName);
    RETURN_VAL_IF_FALSE(port, false);
    return m_context->setSourceActivePort(getIndex(), portName);
}

bool PulseSource::setMute(int32_t mute)
{
    return m_context->setSourceMute(getIndex(), mute);
}

bool PulseSource::setCvolume(const pa_cvolume &cvolume)
{
    return m_context->setSourceVolume(getIndex(), &cvolume);
}

}  // namespace Kiran