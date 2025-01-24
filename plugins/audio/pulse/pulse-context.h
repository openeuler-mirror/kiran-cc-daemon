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

typedef struct pa_glib_mainloop pa_glib_mainloop;
struct pa_mainloop;
struct pa_context;
struct pa_server_info;
struct pa_card_info;
struct pa_sink_info;
struct pa_sink_input_info;
struct pa_source_info;
struct pa_source_output_info;

class QTimer;

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

class PulseContext : public QObject
{
    Q_OBJECT

public:
    PulseContext();
    virtual ~PulseContext();

    /**
     * @brief 连接PulseAudio服务器。
     * @param {bool} waitForDaemon 如果设置为真，则当调用pa_context_connect函数且服务器不可用时不返回失败，而是将状态设置为PA_CONTEXT_CONNECTING并等待服务器恢复正常。
     * @return {bool} 连接错误则返回失败。
     */
    bool connect(bool waitForDaemon);

    // 断开服务器连接
    void disconnect();
    // 获取连接状态
    PulseConnectionState getConnectionState() { return m_connectionState; };
    // 更新服务器信息
    bool loadServerInfo();
    // 更新card信息
    bool loadCardInfo(uint32_t index);
    bool loadCardInfoByName(const QString &name);
    // 更新sink信息
    bool loadSinkInfo(uint32_t index);
    bool loadSinkInfoByName(const QString &name);
    // 更新sink input信息
    bool loadSinkInputInfo(uint32_t index);
    // 更新source信息
    bool loadSourceInfo(uint32_t index);
    bool loadSourceInfoByName(const QString &name);
    // 更新source output信息
    bool loadSourceOutputInfo(uint32_t index);
    // 设置默认sink
    bool setDefaultSink(const QString &name);
    // 设置默认source
    bool setDefaultSource(const QString &name);
    // 设置card profile
    bool setCardProfile(const QString &card, const QString &profile);
    // 设置sink静音
    bool setSinkMute(uint32_t index, int32_t mute);
    // 设置sink音量
    bool setSinkVolume(uint32_t index, const pa_cvolume *volume);
    // 设置sink的active port
    bool setSinkActivePort(uint32_t index, const QString &portName);
    // 设置sink input静音
    bool setSinkInputMute(uint32_t index, int32_t mute);
    // 设置sink input音量
    bool setSinkInputVolume(uint32_t index, const pa_cvolume *volume);
    // 设置source静音
    bool setSourceMute(uint32_t index, int32_t mute);
    // 设置source音量
    bool setSourceVolume(uint32_t index, const pa_cvolume *volume);
    // 设置source的active port
    bool setSourceActivePort(uint32_t index, const QString &portName);
    // 设置source output静音
    bool setSourceOutputMute(uint32_t index, int32_t mute);
    // 设置source output音量
    bool setSourceOutputVolume(uint32_t index, const pa_cvolume *volume);
    // 挂起sink
    bool suspendSink(uint32_t index, bool suspend);
    // 挂起source
    bool suspendSource(uint32_t index, bool suspend);
    // sink input更换新的sink连接
    bool moveSinkInput(uint32_t index, uint32_t sinkIndex);
    // source output更换新的source连接
    bool moveSourceOutput(uint32_t index, uint32_t sourceIndex);
    bool killSinkInput(uint32_t index);
    bool killSourceOutput(uint32_t index);

Q_SIGNALS:
    // 连接状态发生变化
    void connectionStateChanged(int32_t state);
    // 音频相关数据信息发生变化
    void serverInfoChanged(const pa_server_info *info);
    void cardInfoChanged(const pa_card_info *info);
    void cardInfoRemoved(uint32_t index);
    void sinkInfoChanged(const pa_sink_info *info);
    void sinkInfoRemoved(uint32_t index);
    void sinkInputInfoChanged(const pa_sink_input_info *info);
    void sinkInputInfoRemoved(uint32_t index);
    void sourceInfoChanged(const pa_source_info *info);
    void sourceInfoRemoved(uint32_t index);
    void sourceOutputInfoChanged(const pa_source_output_info *info);
    void sourceOutputInfoRemoved(uint32_t index);

private:
    // void iteratePulseEvent();
    // 设置连接状态
    void setConnectionState(PulseConnectionState new_state);

    bool loadLists();
    bool loadListFinished();

    bool processPulseOperation(pa_operation *op);

    // 连接PulseAudio服务器状态变化的回调函数
    static void processStateChanged(pa_context *context, void *userdata);
    // 订阅通知回调函数
    static void processSubscribeEvent(pa_context *context, pa_subscription_event_type_t event_type, uint32_t idx, void *userdata);
    // 获取服务器信息回调
    static void processServerInfo(pa_context *context, const pa_server_info *serverInfo, void *userdata);
    // 获取card信息回调
    static void processCardInfo(pa_context *context, const pa_card_info *cardInfo, int eol, void *userdata);
    // 获取sink信息回调
    static void processSinkInfo(pa_context *context, const pa_sink_info *sinkInfo, int eol, void *userdata);
    // 获取sink input信息回调
    static void processSinkInputInfo(pa_context *context, const pa_sink_input_info *sinkInputInfo, int eol, void *userdata);
    // 获取source信息回调
    static void processSourceInfo(pa_context *context, const pa_source_info *sourceInfo, int eol, void *userdata);
    // 获取source output信息回调
    static void processSourceOutputInfo(pa_context *context, const pa_source_output_info *sourceOutputInfo, int eol, void *userdata);
    // 获取ext stream信息回调
    // static void on_pulse_ext_stream_restore_cb(pa_context *context, const pa_ext_stream_restore_info *ext_stream_restore_info, int eol, void *userdata);

private:
    pa_glib_mainloop *m_mainLoop;
    // 定时处理puseaudio的事件
    QTimer *m_eventTimer;
    // 连接相关的属性
    pa_proplist *m_props;
    // 上下文信息
    pa_context *m_context;
    // 连接状态
    PulseConnectionState m_connectionState;
    // 加载的请求计数
    int32_t m_requestCount;
};
}  // namespace Kiran