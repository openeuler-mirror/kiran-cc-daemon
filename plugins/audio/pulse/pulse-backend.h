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
#include <QMap>
#include <QObject>
#include <QSharedPointer>
#include "audio-i.h"

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

class QTimer;

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
    QString userName;
    // PulseAudio服务器的主机名
    QString hostName;
    // PulseAudio服务器版本
    QString serverVersion;
    // PulseAudio服务器包名（一般未pulseaudio）
    QString serverName;
    // 默认采样策略
    pa_sample_spec sampleSpec;
    // 默认sink名
    QString defaultSinkName;
    // 默认source名
    QString defaultSourceName;
    // 随机数cookie，用于标识一个PulseAudio实例
    uint32_t cookie;
    // pa_channel_map channel_map;
};

class PulseContext;
class PulseCard;
class PulseSink;
class PulseSinkInput;
class PulseSource;
class PulseSourceOutput;

class PulseBackend : public QObject
{
    Q_OBJECT

public:
    PulseBackend();
    virtual ~PulseBackend();

    static PulseBackend *getInstance() { return m_instance; };

    static void globalInit();

    static void globalDeinit() { delete m_instance; };

    QSharedPointer<PulseContext> getContext() { return m_context; };
    AudioState getState() { return m_state; };

    QList<QSharedPointer<PulseCard>> getCards() { return m_cards.values(); };
    QList<QSharedPointer<PulseSink>> getSinks() { return m_sinks.values(); };
    QList<QSharedPointer<PulseSinkInput>> getSinkInputs() { return m_sinkInputs.values(); };
    QList<QSharedPointer<PulseSource>> getSources() { return m_sources.values(); };
    QList<QSharedPointer<PulseSourceOutput>> getSourceOutputs() { return m_sourceOutputs.values(); };

    QSharedPointer<PulseCard> getCard(uint32_t index) { return m_cards.value(index); };
    QSharedPointer<PulseSink> getSink(uint32_t index) { return m_sinks.value(index); };
    QSharedPointer<PulseSink> getSinkByName(const QString &name);
    QSharedPointer<PulseSinkInput> getSinkInput(uint32_t index) { return m_sinkInputs.value(index); };
    QSharedPointer<PulseSource> getSource(uint32_t index) { return m_sources.value(index); };
    QSharedPointer<PulseSource> getSourceByName(const QString &name);
    QSharedPointer<PulseSourceOutput> getSourceOutput(uint32_t index) { return m_sourceOutputs.value(index); };

    bool setDefaultSink(QSharedPointer<PulseSink> sink);
    bool setDefaultSource(QSharedPointer<PulseSource> source);
    QSharedPointer<PulseSink> getDefaultSink() { return m_defaultSink; };
    QSharedPointer<PulseSource> getDefaultSource() { return m_defaultSource; };

Q_SIGNALS:
    void stateChanged(AudioState state);
    void defaultSinkChanged(QSharedPointer<PulseSink> sink);
    void defaultSourceChanged(QSharedPointer<PulseSource> source);
    void cardEvent(PulseCardEvent event, QSharedPointer<PulseCard> card);
    void sinkEvent(PulseSinkEvent event, QSharedPointer<PulseSink> sink);
    void sinkInputEvent(PulseSinkInputEvent event, QSharedPointer<PulseSinkInput> sinkInput);
    void sourceEvent(PulseSourceEvent event, QSharedPointer<PulseSource> source);
    void sourceOutputEvent(PulseSourceOutputEvent event, QSharedPointer<PulseSourceOutput> sourceOutput);

private:
    bool init();

    void setState(AudioState state);
    // 尝试重新连接，直到连接成功为止
    void tryReconnection();
    // 如果断开连接，则重置数据
    void resetData();

    void processConnectionStateChanged(int32_t connectionState);
    void processServerInfoChanged(const pa_server_info *serverInfo);
    void processCardInfoChanged(const pa_card_info *cardInfo);
    void processCardInfoRemoved(uint32_t index);
    void processSinkInfoChanged(const pa_sink_info *sinkInfo);
    void processSinkInfoRemoved(uint32_t index);
    void processSinkInputInfoChanged(const pa_sink_input_info *sinkInputInfo);
    void processSinkInputInfoRemoved(uint32_t index);
    void processSourceInfoChanged(const pa_source_info *sourceInfo);
    void processSourceInfoRemoved(uint32_t index);
    void processSourceOutputInfoChanged(const pa_source_output_info *sourceOutputInfo);
    void processSourceOutputInfoRemoved(uint32_t index);

private:
    static PulseBackend *m_instance;

    QSharedPointer<PulseContext> m_context;

    // 可用状态
    AudioState m_state;
    QTimer *m_reconnectTimer;
    // 重新连接次数
    int32_t m_reconnectionCount;
    uint32_t m_reconnectionHandle;

    PulseServerInfo m_serverInfo;
    QMap<uint32_t, QSharedPointer<PulseCard>> m_cards;
    QMap<uint32_t, QSharedPointer<PulseSink>> m_sinks;
    QMap<uint32_t, QSharedPointer<PulseSinkInput>> m_sinkInputs;
    QMap<uint32_t, QSharedPointer<PulseSource>> m_sources;
    QMap<uint32_t, QSharedPointer<PulseSourceOutput>> m_sourceOutputs;

    // default_sink的name在card profile发生变化时可能会在某一刻时间与server_info中的default_sink_name不相等。default source同理
    QSharedPointer<PulseSink> m_defaultSink;
    QSharedPointer<PulseSource> m_defaultSource;
};
}  // namespace Kiran