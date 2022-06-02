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

#include "plugins/audio/pulse/pulse-node.h"

namespace Kiran
{
struct PulseStreamInfo : public PulseNodeInfo
{
public:
    PulseStreamInfo(const pa_sink_input_info *sink_input_info);
    PulseStreamInfo(const pa_source_output_info *source_output_info);

    int has_volume;
    int volume_writable;
};

class PulseStream : public PulseNode
{
public:
    PulseStream(const PulseStreamInfo &stream_info);
    virtual ~PulseStream(){};

protected:
    void update(const PulseStreamInfo &stream_info);
};
}  // namespace Kiran