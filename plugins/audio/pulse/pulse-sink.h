/**
 * @file          /kiran-cc-daemon/plugins/audio/pulse/pulse-sink.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */
#pragma once

#include "plugins/audio/pulse/pulse-device.h"

namespace Kiran
{
class PulseContext;

class PulseSink : public PulseDevice
{
public:
    PulseSink(std::shared_ptr<PulseContext> context, const pa_sink_info *sink_info);
    virtual ~PulseSink(){};

    void update(const pa_sink_info *sink_info);

    // 设置活动的端口
    virtual bool set_active_port(const std::string &port_name) override;
    virtual bool set_mute(int32_t mute) override;
    virtual bool set_cvolume(const pa_cvolume &cvolume) override;

private:
    std::shared_ptr<PulseContext> context_;
};

using PulseSinkVec = std::vector<std::shared_ptr<PulseSink>>;
}  // namespace Kiran