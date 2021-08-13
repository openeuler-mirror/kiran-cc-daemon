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

#include "plugins/audio/pulse/pulse-node.h"

namespace Kiran
{
struct PulseStreamInfo
{
public:
    PulseStreamInfo(const pa_sink_input_info *sink_input_info);
    PulseStreamInfo(const pa_source_output_info *source_output_info);

    uint32_t index;
    std::string name;
    pa_channel_map channel_map;
    pa_cvolume cvolume;
    int32_t mute;
    int has_volume;
    int volume_writable;
    std::string application_name;
    std::string icon_name;
};

class PulseStream : public PulseNode
{
public:
    PulseStream(const PulseStreamInfo &stream_info);
    virtual ~PulseStream(){};

    std::string get_application_name() { return this->application_name_; };
    std::string get_icon_name() { return this->icon_name_; };

    sigc::signal<void, const std::string &> &signal_icon_name_changed() { return this->icon_name_changed_; };

protected:
    void update(const PulseStreamInfo &stream_info);

private:
    std::string application_name_;
    std::string icon_name_;

    sigc::signal<void, const std::string &> icon_name_changed_;
};
}  // namespace Kiran