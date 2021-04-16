/**
 * @file          /kiran-cc-daemon/plugins/audio/pulse/pulse-card-port.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */
#pragma once

#include "plugins/audio/pulse/pulse-port.h"

namespace Kiran
{
class PulseCardPort : public PulsePort
{
public:
    PulseCardPort(const pa_card_port_info *card_port_info);
    virtual ~PulseCardPort(){};
};

using PulseCardPortVec = std::vector<std::shared_ptr<PulseCardPort>>;
}  // namespace Kiran