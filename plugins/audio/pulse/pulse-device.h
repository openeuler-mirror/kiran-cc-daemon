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

#pragma once

#include <QMap>
#include <QSharedPointer>
#include "pulse-node.h"
#include "pulse-port.h"

namespace Kiran
{
struct PulseDeviceInfo : public PulseNodeInfo
{
public:
    PulseDeviceInfo(const pa_sink_info *sinkInfo);
    PulseDeviceInfo(const pa_source_info *sourceInfo);

    // 设备对应的声卡
    uint32_t m_cardIndex;
    // 设备可用的端口
    QMap<QString, QSharedPointer<PulsePort>> m_ports;
    // 活动端口
    QString m_activePortName;
};

class PulseDevice : public PulseNode
{
    Q_OBJECT

public:
    PulseDevice(const PulseDeviceInfo &deviceInfo);
    virtual ~PulseDevice() {};

    // 设置活动的端口
    virtual bool setActivePort(const QString &portName);

    // 获取活动的端口
    QString getActivePort() { return m_activePortName; };
    // 根据名字获取端口
    QSharedPointer<PulsePort> get_port(const QString &portName)
    {
        return m_ports.value(portName);
    };
    // 获取所有绑定端口
    PulsePortVec getPorts() { return m_ports.values(); };
    // 获取声卡索引
    uint32_t getCardIndex() { return m_cardIndex; };

Q_SIGNALS:
    // 相关信号
    void activePortChanged(const QString &portName);

protected:
    void update(const PulseDeviceInfo &deviceInfo);

private:
    // card index
    uint32_t m_cardIndex;
    // sink ports
    QMap<QString, QSharedPointer<PulsePort>> m_ports;
    // active sink port
    QString m_activePortName;
};
}  // namespace Kiran