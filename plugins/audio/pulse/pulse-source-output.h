/**
 * @file          /kiran-cc-daemon/plugins/audio/pulse/pulse-source-output.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#pragma once

#include <pulse/introspect.h>

#include "plugins/audio/pulse/pulse-stream.h"

namespace Kiran
{
class PulseContext;

class PulseSourceOutput : public PulseStream
{
public:
    PulseSourceOutput(std::shared_ptr<PulseContext> context,
                      const pa_source_output_info *source_output_info);
    virtual ~PulseSourceOutput(){};

    void update(const pa_source_output_info *source_output_info);

    virtual bool set_mute(int32_t mute) override;
    virtual bool set_cvolume(const pa_cvolume &cvolume) override;

private:
    std::shared_ptr<PulseContext> context_;
};

using PulseSourceOutputVec = std::vector<std::shared_ptr<PulseSourceOutput>>;
}  // namespace Kiran