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

#include <pulse/introspect.h>
#include <QObject>

namespace Kiran
{
class PulsePort : public QObject
{
    Q_OBJECT

public:
    PulsePort(const pa_sink_port_info *sinkPortInfo);
    PulsePort(const pa_source_port_info *sourcePortInfo);
    PulsePort(const QString &name,
              const QString &description,
              uint32_t priority,
              int32_t available);

    virtual ~PulsePort(){};

    const QString &getName() const { return m_name; };
    const QString &getDescription() const { return m_description; };
    uint32_t getPriority() const { return m_priority; };
    int32_t getAvailable() const { return m_available; };

private:
    // 端口名字
    QString m_name;
    // 端口描述
    QString m_description;
    // 优先级越高，表示越适合作为默认端口
    uint32_t m_priority;
    // 端口的可用状态，参考pa_port_available定义
    int m_available;
};

using PulsePortVec = QList<QSharedPointer<PulsePort>>;
}  // namespace Kiran