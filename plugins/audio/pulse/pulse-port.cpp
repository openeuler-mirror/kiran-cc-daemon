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

#include "pulse-port.h"
#include "lib/base/base.h"

namespace Kiran
{
PulsePort::PulsePort(const pa_sink_port_info *sinkPortInfo) : PulsePort(POINTER_TO_STRING(sinkPortInfo->name),
                                                                        POINTER_TO_STRING(sinkPortInfo->description),
                                                                        sinkPortInfo->priority,
                                                                        sinkPortInfo->available)
{
}

PulsePort::PulsePort(const pa_source_port_info *sourcePortInfo) : PulsePort(POINTER_TO_STRING(sourcePortInfo->name),
                                                                            POINTER_TO_STRING(sourcePortInfo->description),
                                                                            sourcePortInfo->priority,
                                                                            sourcePortInfo->available)
{
}

PulsePort::PulsePort(const QString &name,
                     const QString &description,
                     uint32_t priority,
                     int32_t available) : m_name(name),
                                          m_description(description),
                                          m_priority(priority),
                                          m_available(available)
{
    KLOG_DEBUG(audio) << "The name is" << name
                      << ", it's description is about " << description
                      << ", it's priority is " << priority
                      << " and it's available is " << available;
}
}  // namespace Kiran