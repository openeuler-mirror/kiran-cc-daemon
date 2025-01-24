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

#include "pulse-device.h"
#include "lib/base/base.h"

namespace Kiran
{
PulseDeviceInfo::PulseDeviceInfo(const pa_sink_info *sinkInfo) : PulseNodeInfo(PulseNodeInfo{.index = sinkInfo->index,
                                                                                             .name = POINTER_TO_STRING(sinkInfo->name),
                                                                                             .channelMap = sinkInfo->channel_map,
                                                                                             .cvolume = sinkInfo->volume,
                                                                                             .mute = sinkInfo->mute,
                                                                                             .baseVolume = sinkInfo->base_volume,
                                                                                             .proplist = sinkInfo->proplist}),

                                                                 m_cardIndex(sinkInfo->card)
{
    for (uint32_t i = 0; i < sinkInfo->n_ports; ++i)
    {
        auto port = QSharedPointer<PulsePort>::create(sinkInfo->ports[i]);
        auto portName = port->getName();
        if (m_ports.contains(portName))
        {
            KLOG_WARNING(audio) << "The port" << portName << "already exist.";
        }
        else
        {
            m_ports.insert(portName, port);
        }
    }

    if (sinkInfo->active_port)
    {
        m_activePortName = POINTER_TO_STRING(sinkInfo->active_port->name);
    }
}

PulseDeviceInfo::PulseDeviceInfo(const pa_source_info *sourceInfo) : PulseNodeInfo(PulseNodeInfo{.index = sourceInfo->index,
                                                                                                 .name = POINTER_TO_STRING(sourceInfo->name),
                                                                                                 .channelMap = sourceInfo->channel_map,
                                                                                                 .cvolume = sourceInfo->volume,
                                                                                                 .mute = sourceInfo->mute,
                                                                                                 .baseVolume = sourceInfo->base_volume,
                                                                                                 .proplist = sourceInfo->proplist}),
                                                                     m_cardIndex(sourceInfo->card)
{
    for (uint32_t i = 0; i < sourceInfo->n_ports; ++i)
    {
        auto port = QSharedPointer<PulsePort>::create(sourceInfo->ports[i]);
        auto portName = port->getName();
        if (m_ports.contains(portName))
        {
            KLOG_WARNING(audio) << "The port" << portName << "already exist.";
        }
        else
        {
            m_ports.insert(portName, port);
        }
    }

    if (sourceInfo->active_port)
    {
        m_activePortName = POINTER_TO_STRING(sourceInfo->active_port->name);
    }
}

PulseDevice::PulseDevice(const PulseDeviceInfo &deviceInfo) : PulseNode(deviceInfo),
                                                              m_cardIndex(deviceInfo.m_cardIndex),
                                                              m_ports(deviceInfo.m_ports),
                                                              m_activePortName(deviceInfo.m_activePortName)
{
}

bool PulseDevice::setActivePort(const QString &portName)
{
    // 不支持，需要调用子类函数
    return false;
}

void PulseDevice::update(const PulseDeviceInfo &deviceInfo)
{
    m_ports = deviceInfo.m_ports;

    if (m_activePortName != deviceInfo.m_activePortName)
    {
        m_activePortName = deviceInfo.m_activePortName;
        Q_EMIT activePortChanged(m_activePortName);
    }

    PulseNode::update(deviceInfo.channelMap, deviceInfo.cvolume, deviceInfo.mute, deviceInfo.baseVolume);
}
}  // namespace Kiran