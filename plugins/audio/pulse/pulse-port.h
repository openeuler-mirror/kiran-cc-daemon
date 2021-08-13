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