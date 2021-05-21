/**
 * @file          /kiran-cc-daemon/plugins/audio/pulse/pulse-node.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#pragma once

#include <pulse/introspect.h>
#include "audio-i.h"
#include "lib/base/base.h"

/*
mute ------ base ------ norm ------ norm * n (max)
mute : 静音，一般对应0分贝
base : 依赖于硬件设备
norm : 硬件设备支持的最大音量
norm * n (max) : 超过硬件的最大音量，PA(PulseAudio)会进行数字放大，仅仅适用于已经设置PA_SINK_DECIBEL_VOLUME/PA_SOURCE_DECIBEL_VOLUME的设备
*/

namespace Kiran
{
class PulseCard;

// PulseStream字段变化信号的标记
enum PulseNodeField
{
    PULSE_NODE_FIELD_MUTE = 0,
    PULSE_NODE_FIELD_VOLUME,
    PULSE_NODE_FIELD_BALANCE,
    PULSE_NODE_FIELD_FADE,
};

class PulseNode
{
public:
    PulseNode(uint32_t index,
              const std::string &name,
              const pa_channel_map &channel_map,
              const pa_cvolume &cvolume,
              int32_t mute,
              pa_volume_t base_volume);

    virtual ~PulseNode(){};

    uint32_t get_index() { return this->index_; };
    const std::string &get_name() { return this->name_; };

    // 获取静音
    bool get_mute() { return this->mute_; };
    // 获取音量
    uint32_t get_volume() { return this->volume_; };
    // 获取左声道和右声道的平衡
    float get_balance() { return this->balance_; };
    // 获取前声道和后声道的平衡
    float get_fade() { return this->fade_; }
    //
    AudioNodeState get_flags() { return this->flags_; };

    // 设置静音
    bool set_mute(bool mute);
    // 设置音量
    bool set_volume(uint32_t volume);
    // 设置左右平衡，balance的范围为[-1, 1]
    bool set_balance(float balance);
    // 设置前后平衡，范围为[-1, 1]
    bool set_fade(float fade);

    // 获取最小音量
    uint32_t get_min_volume() { return uint32_t(PA_VOLUME_MUTED); };
    // 获取最大音量
    uint32_t get_max_volume();
    // 获取正常音量
    uint32_t get_normal_volume() { return uint32_t(PA_VOLUME_NORM); };
    // base音量
    uint32_t get_base_volume();

    sigc::signal<void, PulseNodeField> &signal_node_info_changed() { return this->node_info_changed_; };

protected:
    void update(const pa_channel_map &channel_map,
                const pa_cvolume &cvolume,
                int32_t mute,
                pa_volume_t base_volume);

    virtual bool set_mute(int32_t mute);
    virtual bool set_cvolume(const pa_cvolume &cvolume);
    // 构造函数中需要调用，不能设为虚函数
    void update_flags();

private:
    void update_mute(bool mute);
    void update_volume(uint32_t volume);
    void update_banlance(float balance);
    void update_fade(float fade);
    bool update_cvolume(const pa_cvolume &cvolume);

protected:
    AudioNodeState flags_;

private:
    uint32_t index_;
    std::string name_;
    pa_channel_map channel_map_;
    pa_cvolume cvolume_;
    pa_volume_t base_volume_;

    // 静音
    bool mute_;
    // 音量
    uint32_t volume_;
    // 左右声道的平衡
    float balance_;
    // 前后声道的平衡
    float fade_;

    sigc::signal<void, PulseNodeField> node_info_changed_;
};

using PulseStreamVec = std::vector<std::shared_ptr<PulseNode>>;
}  // namespace Kiran