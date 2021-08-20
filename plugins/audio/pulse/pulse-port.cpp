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

#include "plugins/audio/pulse/pulse-port.h"

namespace Kiran
{
PulsePort::PulsePort(const pa_sink_port_info *sink_port_info) : PulsePort(POINTER_TO_STRING(sink_port_info->name),
                                                                          POINTER_TO_STRING(sink_port_info->description),
                                                                          sink_port_info->priority,
                                                                          sink_port_info->available)
{
}

PulsePort::PulsePort(const pa_source_port_info *source_port_info) : PulsePort(POINTER_TO_STRING(source_port_info->name),
                                                                              POINTER_TO_STRING(source_port_info->description),
                                                                              source_port_info->priority,
                                                                              source_port_info->available)
{
}

PulsePort::PulsePort(const std::string &name,
                     const std::string &description,
                     uint32_t priority,
                     int32_t available) : name_(name),
                                          description_(description),
                                          priority_(priority),
                                          available_(available)
{
    KLOG_DEBUG("name: %s, description: %s,  priority: %d, available: %d.",
               name.c_str(),
               description.c_str(),
               priority,
               available);
}
}  // namespace Kiran