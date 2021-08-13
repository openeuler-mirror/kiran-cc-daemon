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

#include "lib/base/base.h"

#include "audio-i.h"
#include "plugins/audio/pulse/pulse-card.h"
#include "plugins/audio/pulse/pulse-context.h"
#include "plugins/audio/pulse/pulse-sink-input.h"
#include "plugins/audio/pulse/pulse-sink.h"
#include "plugins/audio/pulse/pulse-source-output.h"
#include "plugins/audio/pulse/pulse-source.h"

/* 
1个主机可以有多个card，一个card可以看作是一个声卡
一个card可以有多个card profile，card profile可以看作是card的一种配置方案，同一时间只有一个card profile在使用
一个card profile可以有多个source和多个sink
一个source可以拥有多个device port，同一时间只有一个激活(active)的port
一个sink可以拥有多个device port，同一时间只有一个激活(active)的port

microphone --> source --> source output   |  sink input --> sink --> headphones

                          -----------------------------
                          | --> source --> device port |
card --> card profile --> |                            |
                          | -->  sink  --> device port |
                          -----------------------------

*/

namespace Kiran
{
enum PulseCardEvent
{
    PULSE_CARD_EVENT_ADDED,
    PULSE_CARD_EVENT_DELETED,
    PULSE_CARD_EVENT_CHANGED
};

enum PulseSinkEvent
{
    PULSE_SINK_EVENT_ADDED,
    PULSE_SINK_EVENT_DELETED,
    PULSE_SINK_EVENT_CHANGED
};

enum PulseSourceEvent
{
    PULSE_SOURCE_EVENT_ADDED,
    PULSE_SOURCE_EVENT_DELETED,
    PULSE_SOURCE_EVENT_CHANGED
};

enum PulseSinkInputEvent
{
    PULSE_SINK_INPUT_EVENT_ADDED,
    PULSE_SINK_INPUT_EVENT_DELETED,
    PULSE_SINK_INPUT_EVENT_CHANGED
};

enum PulseSourceOutputEvent
{
    PULSE_SOURCE_OUTPUT_EVENT_ADDED,
    PULSE_SOURCE_OUTPUT_EVENT_DELETED,
    PULSE_SOURCE_OUTPUT_EVENT_CHANGED
};

struct PulseServerInfo
{
    // 启动PulseAudio服务器的用户名
    std::string user_name;
    // PulseAudio服务器的主机名
    std::string host_name;
    // PulseAudio服务器版本
    std::string server_version;
    // PulseAudio服务器包名（一般未pulseaudio）
    std::string server_name;
    // 默认采样策略
    pa_sample_spec sample_spec;
    // 默认sink名
    std::string default_sink_name;
    // 默认source名
    std::string default_source_name;
    // 随机数cookie，用于标识一个PulseAudio实例
    uint32_t cookie;
    // pa_channel_map channel_map;
};

class PulseBackend
{
public:
    PulseBackend();
    virtual ~PulseBackend();

    static PulseBackend *get_instance() { return instance_; };

    static void global_init();

    static void global_deinit() { delete instance_; };

    std::shared_ptr<PulseContext> get_context() { return this->context_; };
    AudioState get_state() { return this->state_; };

    PulseCardVec get_cards() { return MapHelper::get_values(this->cards_); };
    PulseSinkVec get_sinks() { return MapHelper::get_values(this->sinks_); };
    PulseSinkInputVec get_sink_inputs() { return MapHelper::get_values(this->sink_inputs_); };
    PulseSourceVec get_sources() { return MapHelper::get_values(this->sources_); };
    PulseSourceOutputVec get_source_outputs() { return MapHelper::get_values(this->source_outputs_); };

    std::shared_ptr<PulseCard> get_card(uint32_t index) { return MapHelper::get_value(this->cards_, index); };
    std::shared_ptr<PulseSink> get_sink(uint32_t index) { return MapHelper::get_value(this->sinks_, index); };
    std::shared_ptr<PulseSink> get_sink_by_name(const std::string &name);
    std::shared_ptr<PulseSinkInput> get_sink_input(uint32_t index) { return MapHelper::get_value(this->sink_inputs_, index); };
    std::shared_ptr<PulseSource> get_source(uint32_t index) { return MapHelper::get_value(this->sources_, index); };
    std::shared_ptr<PulseSource> get_source_by_name(const std::string &name);
    std::shared_ptr<PulseSourceOutput> get_source_output(uint32_t index) { return MapHelper::get_value(this->source_outputs_, index); };

    bool set_default_sink(std::shared_ptr<PulseSink> sink);
    bool set_default_source(std::shared_ptr<PulseSource> source);
    std::shared_ptr<PulseSink> get_default_sink() { return this->default_sink_; };
    std::shared_ptr<PulseSource> get_default_source() { return this->default_source_; };

    sigc::signal<void, AudioState> &signal_state_changed() { return this->state_changed_; };
    sigc::signal<void, std::shared_ptr<PulseSink>> &signal_default_sink_changed() { return this->default_sink_changed_; };
    sigc::signal<void, std::shared_ptr<PulseSource>> &signal_default_source_changed() { return this->default_source_changed_; };
    sigc::signal<void, PulseCardEvent, std::shared_ptr<PulseCard>> &signal_card_event() { return this->card_event_; };
    sigc::signal<void, PulseSinkEvent, std::shared_ptr<PulseSink>> &signal_sink_event() { return this->sink_event_; };
    sigc::signal<void, PulseSinkInputEvent, std::shared_ptr<PulseSinkInput>> &signal_sink_input_event() { return this->sink_input_event_; };
    sigc::signal<void, PulseSourceEvent, std::shared_ptr<PulseSource>> &signal_source_event() { return this->source_event_; };
    sigc::signal<void, PulseSourceOutputEvent, std::shared_ptr<PulseSourceOutput>> &signal_source_output_event() { return this->source_output_event_; };

private:
    bool init();

    void set_state(AudioState state);
    // 尝试重新连接，直到连接成功为止
    bool try_reconnection();
    // 如果断开连接，则重置数据
    void reset_data();

    void on_connection_state_changed_cb(PulseConnectionState connection_state);
    void on_server_info_changed_cb(const pa_server_info *server_info);
    void on_card_info_changed_cb(const pa_card_info *card_info);
    void on_card_info_removed_cb(uint32_t index);
    void on_sink_info_changed_cb(const pa_sink_info *sink_info);
    void on_sink_info_removed_cb(uint32_t index);
    void on_sink_input_info_changed_cb(const pa_sink_input_info *sink_input_info);
    void on_sink_input_info_removed_cb(uint32_t index);
    void on_source_info_changed_cb(const pa_source_info *source_info);
    void on_source_info_removed_cb(uint32_t index);
    void on_source_output_info_changed_cb(const pa_source_output_info *source_output_info);
    void on_source_output_info_removed_cb(uint32_t index);

private:
    static PulseBackend *instance_;

    std::shared_ptr<PulseContext> context_;

    // 可用状态
    AudioState state_;
    // 是否成功连接过一次
    bool connected_once_;
    uint32_t reconnection_handle_;

    PulseServerInfo server_info_;
    std::map<uint32_t, std::shared_ptr<PulseCard>> cards_;
    std::map<uint32_t, std::shared_ptr<PulseSink>> sinks_;
    std::map<uint32_t, std::shared_ptr<PulseSinkInput>> sink_inputs_;
    std::map<uint32_t, std::shared_ptr<PulseSource>> sources_;
    std::map<uint32_t, std::shared_ptr<PulseSourceOutput>> source_outputs_;

    // default_sink的name在card profile发生变化时可能会在某一刻时间与server_info中的default_sink_name不相等。default source同理
    std::shared_ptr<PulseSink> default_sink_;
    std::shared_ptr<PulseSource> default_source_;

    sigc::signal<void, AudioState> state_changed_;
    sigc::signal<void, std::shared_ptr<PulseSink>> default_sink_changed_;
    sigc::signal<void, std::shared_ptr<PulseSource>> default_source_changed_;
    sigc::signal<void, PulseCardEvent, std::shared_ptr<PulseCard>> card_event_;
    sigc::signal<void, PulseSinkEvent, std::shared_ptr<PulseSink>> sink_event_;
    sigc::signal<void, PulseSinkInputEvent, std::shared_ptr<PulseSinkInput>> sink_input_event_;
    sigc::signal<void, PulseSourceEvent, std::shared_ptr<PulseSource>> source_event_;
    sigc::signal<void, PulseSourceOutputEvent, std::shared_ptr<PulseSourceOutput>> source_output_event_;
};
}  // namespace Kiran