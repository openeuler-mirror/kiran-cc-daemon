/**
 * @Copyright (C) 2020 ~ 2021 KylinSec Co., Ltd. 
 *
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; If not, see <http: //www.gnu.org/licenses/>. 
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