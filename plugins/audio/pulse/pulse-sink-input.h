/**
 * @file          /kiran-cc-daemon/plugins/audio/pulse/pulse-sink-input.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */
#pragma once

#include <pulse/introspect.h>
#include "plugins/audio/pulse/pulse-stream.h"

namespace Kiran
{
class PulseSink;
class PulseContext;

class PulseSinkInput : public PulseStream
{
public:
    PulseSinkInput(std::shared_ptr<PulseContext> context,
                   const pa_sink_input_info *sink_input_info);
    virtual ~PulseSinkInput(){};

    void update(const pa_sink_input_info *sink_input_info);

    virtual bool set_mute(int32_t mute) override;
    virtual bool set_cvolume(const pa_cvolume &cvolume) override;

private:
    std::shared_ptr<PulseSink> sink_;

    std::shared_ptr<PulseContext> context_;
};

using PulseSinkInputVec = std::vector<std::shared_ptr<PulseSinkInput>>;
}  // namespace Kiran