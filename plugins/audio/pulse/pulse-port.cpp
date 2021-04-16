/**
 * @file          /kiran-cc-daemon/plugins/audio/pulse/pulse-port.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
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
    LOG_DEBUG("name: %s, description: %s,  priority: %d, available: %d.",
              name.c_str(),
              description.c_str(),
              priority,
              available);
}
}  // namespace Kiran