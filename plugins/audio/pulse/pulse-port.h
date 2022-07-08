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

#include <pulse/introspect.h>
#include "lib/base/base.h"

namespace Kiran
{
class PulsePort
{
public:
    PulsePort(const pa_sink_port_info *sink_port_info);
    PulsePort(const pa_source_port_info *source_port_info);
    PulsePort(const std::string &name,
              const std::string &description,
              uint32_t priority,
              int32_t available);

    virtual ~PulsePort(){};

    const std::string &get_name() const { return this->name_; };
    const std::string &get_description() const { return this->description_; };
    uint32_t get_priority() const { return this->priority_; };
    int32_t get_available() const { return this->available_; };

private:
    // 端口名字
    std::string name_;
    // 端口描述
    std::string description_;
    // 优先级越高，表示越适合作为默认端口
    uint32_t priority_;
    // 端口的可用状态，参考pa_port_available定义
    int available_;
};

using PulsePortVec = std::vector<std::shared_ptr<PulsePort>>;
}  // namespace Kiran