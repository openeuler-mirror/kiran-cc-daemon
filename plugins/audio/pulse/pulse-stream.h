/**
 * @file          /kiran-cc-daemon/plugins/audio/pulse/pulse-stream.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
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