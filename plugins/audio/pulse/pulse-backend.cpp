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

#include "pulse-backend.h"
#include <QTimer>
#include "lib/base/base.h"
#include "pulse-card.h"
#include "pulse-context.h"
#include "pulse-sink-input.h"
#include "pulse-sink.h"
#include "pulse-source-output.h"
#include "pulse-source.h"

namespace Kiran
{
#define MAX_RECONNECTION_NUM 50

PulseBackend::PulseBackend() : m_state(AudioState::AUDIO_STATE_IDLE),
                               m_reconnectTimer(nullptr),
                               m_reconnectionCount(0),
                               m_reconnectionHandle(0)
{
    m_context = QSharedPointer<PulseContext>::create();
    m_reconnectTimer = new QTimer(this);
    m_reconnectTimer->setInterval(400);
}

PulseBackend::~PulseBackend()
{
}

PulseBackend *PulseBackend::m_instance = nullptr;
void PulseBackend::globalInit()
{
    m_instance = new PulseBackend();
    m_instance->init();
}

QSharedPointer<PulseSink> PulseBackend::getSinkByName(const QString &name)
{
    for (auto sink : m_sinks)
    {
        if (sink->getName() == name)
        {
            return sink;
        }
    }
    return nullptr;
}

QSharedPointer<PulseSource> PulseBackend::getSourceByName(const QString &name)
{
    for (auto &source : m_sources)
    {
        if (source->getName() == name)
        {
            return source;
        }
    }
    return nullptr;
}

bool PulseBackend::setDefaultSink(QSharedPointer<PulseSink> sink)
{
    RETURN_VAL_IF_FALSE(sink, false);

    RETURN_VAL_IF_FALSE(m_context->setDefaultSink(sink->getName()), false);

    // PULSE_SET_PENDING_SINK_NULL(pulse);
    // PULSE_SET_DEFAULT_SINK(pulse, stream);

    return true;
}

bool PulseBackend::setDefaultSource(QSharedPointer<PulseSource> source)
{
    RETURN_VAL_IF_FALSE(source, false);

    RETURN_VAL_IF_FALSE(m_context->setDefaultSource(source->getName()), false);

    // PULSE_SET_PENDING_SINK_NULL(pulse);
    // PULSE_SET_DEFAULT_SINK(pulse, stream);

    return true;
}

bool PulseBackend::init()
{
    QObject::connect(m_context.data(), &PulseContext::connectionStateChanged, this, &PulseBackend::processConnectionStateChanged);
    QObject::connect(m_context.data(), &PulseContext::serverInfoChanged, this, &PulseBackend::processServerInfoChanged);
    QObject::connect(m_context.data(), &PulseContext::cardInfoChanged, this, &PulseBackend::processCardInfoChanged);
    QObject::connect(m_context.data(), &PulseContext::cardInfoRemoved, this, &PulseBackend::processCardInfoRemoved);
    QObject::connect(m_context.data(), &PulseContext::sinkInfoChanged, this, &PulseBackend::processSinkInfoChanged);
    QObject::connect(m_context.data(), &PulseContext::sinkInfoRemoved, this, &PulseBackend::processSinkInfoRemoved);
    QObject::connect(m_context.data(), &PulseContext::sinkInputInfoChanged, this, &PulseBackend::processSinkInputInfoChanged);
    QObject::connect(m_context.data(), &PulseContext::sinkInputInfoRemoved, this, &PulseBackend::processSinkInputInfoRemoved);
    QObject::connect(m_context.data(), &PulseContext::sourceInfoChanged, this, &PulseBackend::processSourceInfoChanged);
    QObject::connect(m_context.data(), &PulseContext::sourceInfoRemoved, this, &PulseBackend::processSourceInfoRemoved);
    QObject::connect(m_context.data(), &PulseContext::sourceOutputInfoChanged, this, &PulseBackend::processSourceOutputInfoChanged);
    QObject::connect(m_context.data(), &PulseContext::sourceOutputInfoRemoved, this, &PulseBackend::processSourceOutputInfoRemoved);

    QObject::connect(m_reconnectTimer, &QTimer::timeout, this, &PulseBackend::tryReconnection);

    setState(AudioState::AUDIO_STATE_CONNECTING);

    if (!m_context->connect(true))
    {
        setState(AudioState::AUDIO_STATE_FAILED);
        return false;
    }

    return true;
}

void PulseBackend::setState(AudioState state)
{
    if (m_state != state)
    {
        m_state = state;
        Q_EMIT stateChanged(m_state);
    }
}

void PulseBackend::tryReconnection()
{
    ++m_reconnectionCount;

    KLOG_INFO(audio) << "Try to reconnect pulseaudio service, reconnection count:" << m_reconnectionCount;

    if (m_reconnectionCount > MAX_RECONNECTION_NUM)
    {
        KLOG_WARNING(audio) << "The maximum number of reconnections ("
                            << MAX_RECONNECTION_NUM
                            << ") has been exceeded, Stop reconnection.";
        m_reconnectionHandle = 0;
        m_reconnectTimer->stop();
        return;
    }

    if (m_context->connect(true))
    {
        m_reconnectionHandle = 0;
        m_reconnectTimer->stop();
    }
}

void PulseBackend::resetData()
{
    m_serverInfo = PulseServerInfo();

    for (auto card : m_cards)
    {
        Q_EMIT cardEvent(PulseCardEvent::PULSE_CARD_EVENT_DELETED, card);
    }
    m_cards.clear();

    for (auto sink : m_sinks)
    {
        Q_EMIT sinkEvent(PulseSinkEvent::PULSE_SINK_EVENT_DELETED, sink);
    }
    m_sinks.clear();

    for (auto sinkInput : m_sinkInputs)
    {
        Q_EMIT sinkInputEvent(PulseSinkInputEvent::PULSE_SINK_INPUT_EVENT_DELETED, sinkInput);
    }
    m_sinks.clear();

    for (auto source : m_sources)
    {
        Q_EMIT sourceEvent(PulseSourceEvent::PULSE_SOURCE_EVENT_DELETED, source);
    }
    m_sources.clear();

    for (auto sourceOutput : m_sourceOutputs)
    {
        Q_EMIT sourceOutputEvent(PulseSourceOutputEvent::PULSE_SOURCE_OUTPUT_EVENT_DELETED, sourceOutput);
    }
    m_sourceOutputs.clear();
}

void PulseBackend::processConnectionStateChanged(int32_t connectionState)
{
    KLOG_INFO(audio) << "Pulse connection state changed to" << connectionState;

    switch (connectionState)
    {
    case PulseConnectionState::PULSE_CONNECTION_DISCONNECTED:
    {
        // 重新连接之前需要清理掉之前的数据，需要测试一下重启pulseaudio服务程序会不会出问题
        resetData();
        setState(AudioState::AUDIO_STATE_CONNECTING);

        if (m_reconnectTimer->isActive())
        {
            KLOG_INFO(audio) << "The timer is already exist.";
        }
        else
        {
            m_reconnectTimer->start();
        }

        break;
    }
    case PulseConnectionState::PULSE_CONNECTION_CONNECTING:
    case PulseConnectionState::PULSE_CONNECTION_AUTHORIZING:
    case PulseConnectionState::PULSE_CONNECTION_LOADING:
        setState(AudioState::AUDIO_STATE_CONNECTING);
        break;
    case PulseConnectionState::PULSE_CONNECTION_CONNECTED:
    {
        // 如果连接成功，重连次数清0
        m_reconnectionCount = 0;
        setState(AudioState::AUDIO_STATE_READY);
        break;
    }
    default:
        break;
    }
}

void PulseBackend::processServerInfoChanged(const pa_server_info *serverInfo)
{
    RETURN_IF_FALSE(serverInfo != NULL);

    auto oldServerInfo = m_serverInfo;
    m_serverInfo = PulseServerInfo{.userName = POINTER_TO_STRING(serverInfo->user_name),
                                   .hostName = POINTER_TO_STRING(serverInfo->host_name),
                                   .serverVersion = POINTER_TO_STRING(serverInfo->server_version),
                                   .serverName = POINTER_TO_STRING(serverInfo->server_name),
                                   .sampleSpec = serverInfo->sample_spec,
                                   .defaultSinkName = POINTER_TO_STRING(serverInfo->default_sink_name),
                                   .defaultSourceName = POINTER_TO_STRING(serverInfo->default_source_name),
                                   .cookie = serverInfo->cookie};

    KLOG_INFO(audio) << "Server info is changed. Username is" << m_serverInfo.userName
                     << ", hostname is" << m_serverInfo.hostName
                     << ", server version is" << m_serverInfo.serverVersion
                     << ", server name is" << m_serverInfo.serverName
                     << ", default sink name is" << m_serverInfo.defaultSinkName
                     << ", default source name is" << m_serverInfo.defaultSourceName
                     << ", cookie is" << m_serverInfo.cookie;

    // 检测默认的sink是否发生变化
    if (oldServerInfo.defaultSinkName != m_serverInfo.defaultSinkName)
    {
        if (m_serverInfo.defaultSinkName.isEmpty())
        {
            m_defaultSink = nullptr;
            Q_EMIT defaultSinkChanged(m_defaultSink);
        }
        else
        {
            auto sink = getSinkByName(m_serverInfo.defaultSinkName);
            /* 当card profile发生变化时，on_server_info_changed_cb可能优先on_sink_info_changed_cb函数被调用，
               此时default sink可能还找不到（为空），这种情况则将信号延迟到on_sink_info_changed_cb时再进行处理。*/
            if (sink)
            {
                m_defaultSink = sink;
                Q_EMIT defaultSinkChanged(m_defaultSink);
            }
            else
            {
                m_context->loadSinkInfoByName(m_serverInfo.defaultSinkName);
            }
        }
    }

    // 检测默认的source是否发生变化
    if (oldServerInfo.defaultSourceName != m_serverInfo.defaultSourceName)
    {
        if (m_serverInfo.defaultSourceName.isEmpty())
        {
            m_defaultSource = nullptr;
            Q_EMIT defaultSourceChanged(m_defaultSource);
        }
        else
        {
            auto source = getSourceByName(m_serverInfo.defaultSourceName);
            if (source)
            {
                m_defaultSource = source;
                Q_EMIT defaultSourceChanged(m_defaultSource);
            }
            else
            {
                m_context->loadSourceInfoByName(m_serverInfo.defaultSourceName);
            }
        }
    }
}

void PulseBackend::processCardInfoChanged(const pa_card_info *cardInfo)
{
    RETURN_IF_FALSE(cardInfo != NULL);

    KLOG_INFO(audio) << "Card info changed. The card index is" << cardInfo->index
                     << ", name is" << cardInfo->name;

    auto card = getCard(cardInfo->index);

    if (card)
    {
        card->update(cardInfo);
        Q_EMIT cardEvent(PulseCardEvent::PULSE_CARD_EVENT_CHANGED, card);
    }
    else
    {
        card = QSharedPointer<PulseCard>::create(cardInfo);
        m_cards.insert(cardInfo->index, card);
        Q_EMIT cardEvent(PulseCardEvent::PULSE_CARD_EVENT_ADDED, card);
    }
}

void PulseBackend::processCardInfoRemoved(uint32_t index)
{
    KLOG_INFO(audio) << "Remove card with index" << index;

    auto card = getCard(index);

    if (card)
    {
        Q_EMIT cardEvent(PulseCardEvent::PULSE_CARD_EVENT_DELETED, card);
        m_cards.remove(index);
    }
    else
    {
        KLOG_WARNING(audio) << "The card index " << index << " is not found.";
    }
}

void PulseBackend::processSinkInfoChanged(const pa_sink_info *sinkInfo)
{
    RETURN_IF_FALSE(sinkInfo != NULL);

    KLOG_INFO(audio) << "Sink changed, index is" << sinkInfo->index
                     << "and name is" << sinkInfo->name;

    auto sink = getSink(sinkInfo->index);

    if (sink)
    {
        sink->update(sinkInfo);
        Q_EMIT sinkEvent(PulseSinkEvent::PULSE_SINK_EVENT_CHANGED, sink);
    }
    else
    {
        sink = QSharedPointer<PulseSink>::create(m_context, sinkInfo);
        m_sinks.insert(sinkInfo->index, sink);
        Q_EMIT sinkEvent(PulseSinkEvent::PULSE_SINK_EVENT_ADDED, sink);
        /* sink一般情况下都会绑定一个card，部分的后端Device Driver（例如OSS）不存在card的概念，
           因此不确定sink是否在任何情况下都会绑定card，而且card和sink的初始化都是异步回调函数（不确定card是否先于sink回调？)，
           因此保险起见在这里加一个判空条件。*/
        // auto card = get_card(sink_info->card);
        // if (card)
        // {
        //     card->add_stream(sink);
        // }

        // 如果新增的是默认sink，则发送信号（延迟到此时进行处理）
        if (sink->getName() == m_serverInfo.defaultSinkName)
        {
            m_defaultSink = sink;
            Q_EMIT defaultSinkChanged(m_defaultSink);
        }
    }
}

void PulseBackend::processSinkInfoRemoved(uint32_t index)
{
    KLOG_INFO(audio) << "Removed sink info with index" << index;

    auto sink = getSink(index);

    if (!sink)
    {
        KLOG_WARNING(audio) << "The sink index" << index << "is not found.";
        return;
    }

    // auto card = sink->get_card();
    Q_EMIT sinkEvent(PulseSinkEvent::PULSE_SINK_EVENT_DELETED, sink);
    m_sinks.remove(index);

    // if (card)
    // {
    //     card->remove_stream(sink);
    // }

    /*当card profile发生变化时，default sink可能会发生变化，因此这里先进行清理*/
    if (sink->getName() == m_serverInfo.defaultSinkName)
    {
        m_defaultSink = nullptr;
        Q_EMIT defaultSinkChanged(m_defaultSink);
        m_context->loadServerInfo();
    }
}

void PulseBackend::processSinkInputInfoChanged(const pa_sink_input_info *sinkInputInfo)
{
    RETURN_IF_FALSE(sinkInputInfo != NULL);

    KLOG_INFO(audio) << "Sink input changed. Index is" << sinkInputInfo->index
                     << "and name is" << sinkInputInfo->name;

    auto sinkInput = getSinkInput(sinkInputInfo->index);

    if (sinkInput)
    {
        sinkInput->update(sinkInputInfo);
        Q_EMIT sinkInputEvent(PulseSinkInputEvent::PULSE_SINK_INPUT_EVENT_CHANGED, sinkInput);
    }
    else
    {
        sinkInput = QSharedPointer<PulseSinkInput>::create(m_context, sinkInputInfo);
        m_sinkInputs.insert(sinkInputInfo->index, sinkInput);
        Q_EMIT sinkInputEvent(PulseSinkInputEvent::PULSE_SINK_INPUT_EVENT_ADDED, sinkInput);
    }
}

void PulseBackend::processSinkInputInfoRemoved(uint32_t index)
{
    KLOG_INFO(audio) << "Remove sink input with index" << index;

    auto sinkInput = getSinkInput(index);

    if (!sinkInput)
    {
        KLOG_WARNING(audio) << "The sink input index" << index << "is not found.";
        return;
    }

    Q_EMIT sinkInputEvent(PulseSinkInputEvent::PULSE_SINK_INPUT_EVENT_DELETED, sinkInput);
    m_sinkInputs.remove(index);
}

void PulseBackend::processSourceInfoChanged(const pa_source_info *sourceInfo)
{
    RETURN_IF_FALSE(sourceInfo != NULL);

    KLOG_INFO(audio) << "Source changed. Index is" << sourceInfo->index
                     << "and name is" << sourceInfo->name;

    auto source = getSource(sourceInfo->index);

    if (source)
    {
        source->update(sourceInfo);
        Q_EMIT sourceEvent(PulseSourceEvent::PULSE_SOURCE_EVENT_CHANGED, source);
    }
    else
    {
        source = QSharedPointer<PulseSource>::create(m_context, sourceInfo);
        m_sources.insert(sourceInfo->index, source);
        Q_EMIT sourceEvent(PulseSourceEvent::PULSE_SOURCE_EVENT_ADDED, source);

        // auto card = get_card(source_info->card);
        // if (card)
        // {
        //     card->add_stream(source);
        // }

        // 如果新增的是默认source，则发送信号（延迟到此时进行处理）
        if (source->getName() == m_serverInfo.defaultSourceName)
        {
            m_defaultSource = source;
            Q_EMIT defaultSourceChanged(m_defaultSource);
        }
    }
}

void PulseBackend::processSourceInfoRemoved(uint32_t index)
{
    KLOG_INFO(audio) << "Remove source info with index" << index;

    auto source = getSource(index);

    if (!source)
    {
        KLOG_WARNING(audio) << "The source index" << index << "is not found.";
        return;
    }

    // auto card = source->get_card();
    Q_EMIT sourceEvent(PulseSourceEvent::PULSE_SOURCE_EVENT_DELETED, source);
    m_sources.remove(index);

    // if (card)
    // {
    //     card->remove_stream(source);
    // }

    /*当card profile发生变化时，default source可能会发生变化，因此这里先进行清理*/
    if (source->getName() == m_serverInfo.defaultSourceName)
    {
        m_defaultSource = nullptr;
        Q_EMIT defaultSourceChanged(m_defaultSource);
        m_context->loadServerInfo();
    }
}

void PulseBackend::processSourceOutputInfoChanged(const pa_source_output_info *sourceOutputInfo)
{
    RETURN_IF_FALSE(sourceOutputInfo != NULL);

    KLOG_INFO(audio) << "Source output changed. Index is" << sourceOutputInfo->index
                     << "and name is" << sourceOutputInfo->name;

    auto sourceOutput = getSourceOutput(sourceOutputInfo->index);

    if (sourceOutput)
    {
        sourceOutput->update(sourceOutputInfo);
        Q_EMIT sourceOutputEvent(PulseSourceOutputEvent::PULSE_SOURCE_OUTPUT_EVENT_CHANGED, sourceOutput);
    }
    else
    {
        sourceOutput = QSharedPointer<PulseSourceOutput>::create(m_context, sourceOutputInfo);
        m_sourceOutputs.insert(sourceOutputInfo->index, sourceOutput);
        Q_EMIT sourceOutputEvent(PulseSourceOutputEvent::PULSE_SOURCE_OUTPUT_EVENT_ADDED, sourceOutput);
    }
}

void PulseBackend::processSourceOutputInfoRemoved(uint32_t index)
{
    KLOG_INFO(audio) << "Remove source output info with index" << index;

    auto sourceOutput = getSourceOutput(index);

    if (!sourceOutput)
    {
        KLOG_WARNING(audio) << "The source output index" << index << "is not found.";
        return;
    }

    Q_EMIT sourceOutputEvent(PulseSourceOutputEvent::PULSE_SOURCE_OUTPUT_EVENT_DELETED, sourceOutput);
    m_sourceOutputs.remove(index);
}
}  // namespace Kiran