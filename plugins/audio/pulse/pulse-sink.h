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