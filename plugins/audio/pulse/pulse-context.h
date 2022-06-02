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

#include <pulse/ext-stream-restore.h>
#include <pulse/glib-mainloop.h>
#include <pulse/pulseaudio.h>
#include "lib/base/base.h"

namespace Kiran
{
enum PulseConnectionState
{
    PULSE_CONNECTION_DISCONNECTED = 0,
    PULSE_CONNECTION_CONNECTING,
    PULSE_CONNECTION_AUTHORIZING,
    PULSE_CONNECTION_LOADING,
    PULSE_CONNECTION_CONNECTED
};

class PulseContext
{
public:
    PulseContext();
    virtual ~PulseContext();

    /**
     * @brief 连接PulseAudio服务器。
     * @param {bool} wait_for_daemon 如果设置为真，则当调用pa_context_connect函数且服务器不可用时不返回失败，而是将状态设置为PA_CONTEXT_CONNECTING并等待服务器恢复正常。
     * @return {bool} 连接错误则返回失败。
     */
    bool connect(bool wait_for_daemon);

    // 断开服务器连接
    void disconnect();

    // 获取连接状态
    PulseConnectionState get_connection_state() { return this->connection_state_; };

    // 更新服务器信息
    bool load_server_info();
    // 更新card信息
    bool load_card_info(uint32_t index);
    bool load_card_info_by_name(const std::string &name);
    // 更新sink信息
    bool load_sink_info(uint32_t index);
    bool load_sink_info_by_name(const std::string &name);
    // 更新sink input信息
    bool load_sink_input_info(uint32_t index);
    // 更新source信息
    bool load_source_info(uint32_t index);
    bool load_source_info_by_name(const std::string &name);
    // 更新source output信息
    bool load_source_output_info(uint32_t index);
    // 设置默认sink
    bool set_default_sink(const std::string &name);
    // 设置默认source
    bool set_default_source(const std::string &name);
    // 设置card profile
    bool set_card_profile(const std::string &card, const std::string &profile);
    // 设置sink静音
    bool set_sink_mute(uint32_t index, int32_t mute);
    // 设置sink音量
    bool set_sink_volume(uint32_t index, const pa_cvolume *volume);
    // 设置sink的active port
    bool set_sink_active_port(uint32_t index, const std::string &port_name);
    // 设置sink input静音
    bool set_sink_input_mute(uint32_t index, int32_t mute);
    // 设置sink input音量
    bool set_sink_input_volume(uint32_t index, const pa_cvolume *volume);
    // 设置source静音
    bool set_source_mute(uint32_t index, int32_t mute);
    // 设置source音量
    bool set_source_volume(uint32_t index, const pa_cvolume *volume);
    // 设置source的active port
    bool set_source_active_port(uint32_t index, const std::string &port_name);
    // 设置source output静音
    bool set_source_output_mute(uint32_t index, int32_t mute);
    // 设置source output音量
    bool set_source_output_volume(uint32_t index, const pa_cvolume *volume);
    // 挂起sink
    bool suspend_sink(uint32_t index, bool suspend);
    // 挂起source
    bool suspend_source(uint32_t index, bool suspend);
    // sink input更换新的sink连接
    bool move_sink_input(uint32_t index, uint32_t sink_index);
    // source output更换新的source连接
    bool move_source_output(uint32_t index, uint32_t source_index);
    bool kill_sink_input(uint32_t index);
    bool kill_source_output(uint32_t index);

    // 连接状态发生变化
    sigc::signal<void, PulseConnectionState> &signal_connection_state_changed() { return this->connection_state_changed_; };
    // 音频相关数据信息发生变化
    sigc::signal<void, const pa_server_info *> &signal_server_info_changed() { return this->server_info_changed_; };
    sigc::signal<void, const pa_card_info *> &signal_card_info_changed() { return this->card_info_changed_; };
    sigc::signal<void, uint32_t> &signal_card_info_removed() { return this->card_info_removed_; };
    sigc::signal<void, const pa_sink_info *> &signal_sink_info_changed() { return this->sink_info_changed_; };
    sigc::signal<void, uint32_t> &signal_sink_info_removed() { return this->sink_info_removed_; };
    sigc::signal<void, const pa_sink_input_info *> &signal_sink_input_info_changed() { return this->sink_input_info_changed_; };
    sigc::signal<void, uint32_t> &signal_sink_input_info_removed() { return this->sink_input_info_removed_; };
    sigc::signal<void, const pa_source_info *> &signal_source_info_changed() { return this->source_info_changed_; };
    sigc::signal<void, uint32_t> &signal_source_info_removed() { return this->source_info_removed_; };
    sigc::signal<void, const pa_source_output_info *> &signal_source_output_info_changed() { return this->source_output_info_changed_; };
    sigc::signal<void, uint32_t> &signal_source_output_info_removed() { return this->source_output_info_removed_; };

private:
    // 设置连接状态
    void set_connection_state(PulseConnectionState new_state);

    bool load_lists();
    bool load_list_finished();

    bool process_pulse_operation(pa_operation *op);

    // 连接PulseAudio服务器状态变化的回调函数
    static void on_pulse_state_cb(pa_context *context, void *userdata);
    // 订阅通知回调函数
    static void on_pulse_subscribe_cb(pa_context *context, pa_subscription_event_type_t event_type, uint32_t idx, void *userdata);
    // 获取服务器信息回调
    static void on_pulse_server_info_cb(pa_context *context, const pa_server_info *server_info, void *userdata);
    // 获取card信息回调
    static void on_pulse_card_info_cb(pa_context *context, const pa_card_info *card_info, int eol, void *userdata);
    // 获取sink信息回调
    static void on_pulse_sink_info_cb(pa_context *context, const pa_sink_info *sink_info, int eol, void *userdata);
    // 获取sink input信息回调
    static void on_pulse_sink_input_info_cb(pa_context *context, const pa_sink_input_info *sink_input_info, int eol, void *userdata);
    // 获取source信息回调
    static void on_pulse_source_info_cb(pa_context *context, const pa_source_info *source_info, int eol, void *userdata);
    // 获取source output信息回调
    static void on_pulse_source_output_info_cb(pa_context *context, const pa_source_output_info *source_output_info, int eol, void *userdata);
    // 获取ext stream信息回调
    // static void on_pulse_ext_stream_restore_cb(pa_context *context, const pa_ext_stream_restore_info *ext_stream_restore_info, int eol, void *userdata);

    std::string get_default_app_name();

private:
    pa_glib_mainloop *main_loop_;
    // 连接相关的属性
    pa_proplist *props_;
    // 上下文信息
    pa_context *context_;
    // 连接状态
    PulseConnectionState connection_state_;
    // 加载的请求计数
    int32_t request_count_;

    sigc::signal<void, PulseConnectionState> connection_state_changed_;

    sigc::signal<void, const pa_server_info *> server_info_changed_;
    sigc::signal<void, const pa_card_info *> card_info_changed_;
    sigc::signal<void, uint32_t> card_info_removed_;
    sigc::signal<void, const pa_sink_info *> sink_info_changed_;
    sigc::signal<void, uint32_t> sink_info_removed_;
    sigc::signal<void, const pa_sink_input_info *> sink_input_info_changed_;
    sigc::signal<void, uint32_t> sink_input_info_removed_;
    sigc::signal<void, const pa_source_info *> source_info_changed_;
    sigc::signal<void, uint32_t> source_info_removed_;
    sigc::signal<void, const pa_source_output_info *> source_output_info_changed_;
    sigc::signal<void, uint32_t> source_output_info_removed_;
};
}  // namespace Kiran