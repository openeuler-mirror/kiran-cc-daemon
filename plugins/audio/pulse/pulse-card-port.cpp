/**
 * @file          /kiran-cc-daemon/plugins/audio/pulse/pulse-card-port.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#include "plugins/audio/pulse/pulse-card-port.h"

namespace Kiran
{
PulseCardPort::PulseCardPort(const pa_card_port_info *card_port_info) : PulsePort(POINTER_TO_STRING(card_port_info->name),
                                                                                  POINTER_TO_STRING(card_port_info->description),
                                                                                  card_port_info->priority,
                                                                                  card_port_info->available)
{
}
}  // namespace Kiran