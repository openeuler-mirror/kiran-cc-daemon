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

#include "pulse-context.h"
#include <pulse/ext-stream-restore.h>
#include <pulse/glib-mainloop.h>
#include <pulse/pulseaudio.h>
#include <QCoreApplication>
#include <QTimer>
#include "audio-i.h"
#include "config.h"

namespace Kiran
{
PulseContext::PulseContext() : m_mainLoop(NULL),
                               m_eventTimer(nullptr),
                               m_props(NULL),
                               m_context(NULL),
                               m_connectionState(PulseConnectionState::PULSE_CONNECTION_DISCONNECTED),
                               m_requestCount(0)
{
    m_props = pa_proplist_new();
    // 获取QT应用程序名称
    pa_proplist_sets(m_props, PA_PROP_APPLICATION_NAME, QCoreApplication::applicationName().toStdString().c_str());
    pa_proplist_sets(m_props, PA_PROP_APPLICATION_ID, AUDIO_DBUS_NAME);
    pa_proplist_sets(m_props, PA_PROP_APPLICATION_VERSION, PROJECT_VERSION);

    m_mainLoop = pa_glib_mainloop_new(nullptr);

    m_eventTimer = new QTimer(this);
    m_eventTimer->setInterval(100);
    // QObject::connect(m_eventTimer, &QTimer::timeout, this, &PulseContext::iteratePulseEvent);
}

PulseContext::~PulseContext()
{
    if (m_context != NULL)
    {
        pa_context_unref(m_context);
    }

    if (m_props)
    {
        pa_proplist_free(m_props);
    }

    if (m_mainLoop)
    {
        pa_glib_mainloop_free(m_mainLoop);
    }
}

bool PulseContext::connect(bool waitForDaemon)
{
    KLOG_INFO(audio) << "Connect to pulseaudio server which flag waitForDaemon is " << waitForDaemon;

    RETURN_VAL_IF_FALSE(m_mainLoop != NULL, false);

    // 如果已经连接或者正在连接，则直接返回
    RETURN_VAL_IF_TRUE(m_connectionState != PulseConnectionState::PULSE_CONNECTION_DISCONNECTED, true);

    auto mainloop = pa_glib_mainloop_get_api(m_mainLoop);
    m_context = pa_context_new_with_proplist(mainloop, NULL, m_props);
    if (!m_context)
    {
        KLOG_WARNING(audio) << "Failed to create PulseAudio context";
        return false;
    }

    // 连接服务器为异步操作，需要设置回调函数监听连接的状态变化
    pa_context_set_state_callback(m_context, &PulseContext::processStateChanged, this);

    pa_context_flags_t flags = waitForDaemon ? PA_CONTEXT_NOFAIL : PA_CONTEXT_NOFLAGS;
    if (pa_context_connect(m_context, NULL, flags, NULL) == 0)
    {
        setConnectionState(PulseConnectionState::PULSE_CONNECTION_CONNECTING);
        return true;
    }
    else
    {
        KLOG_WARNING(audio) << "Failed to connect pulseaudio service.";
        // on_pulse_state_cb回调函数可能已经进行了释放操作，所以这里需要进一步判断
        if (m_context)
        {
            pa_context_unref(m_context);
            m_context = NULL;
        }
        return false;
    }
}

void PulseContext::disconnect()
{
    RETURN_IF_TRUE(m_connectionState == PulseConnectionState::PULSE_CONNECTION_DISCONNECTED);

    if (m_context != NULL)
    {
        pa_context_unref(m_context);
        m_context = NULL;
    }
    m_requestCount = 0;

    // connection->priv->ext_streams_loading = FALSE;
    // connection->priv->ext_streams_dirty = FALSE;

    setConnectionState(PulseConnectionState::PULSE_CONNECTION_DISCONNECTED);
}

bool PulseContext::loadServerInfo()
{
    RETURN_VAL_IF_FALSE(m_connectionState == PulseConnectionState::PULSE_CONNECTION_LOADING ||
                            m_connectionState == PulseConnectionState::PULSE_CONNECTION_CONNECTED,
                        false);

    auto op = pa_context_get_server_info(m_context, &PulseContext::processServerInfo, this);
    return processPulseOperation(op);
}

bool PulseContext::loadCardInfo(uint32_t index)
{
    pa_operation *op = NULL;

    RETURN_VAL_IF_FALSE(m_connectionState == PulseConnectionState::PULSE_CONNECTION_LOADING ||
                            m_connectionState == PulseConnectionState::PULSE_CONNECTION_CONNECTED,
                        false);

    // 如果未设置index，则获取所有的card信息，否则获取指定的card信息
    if (index != PA_INVALID_INDEX)
    {
        op = pa_context_get_card_info_by_index(m_context, index, &PulseContext::processCardInfo, this);
    }
    else
    {
        op = pa_context_get_card_info_list(m_context, &PulseContext::processCardInfo, this);
    }

    return processPulseOperation(op);
}

bool PulseContext::loadCardInfoByName(const QString &name)
{
    RETURN_VAL_IF_FALSE(!name.isEmpty(), false);

    RETURN_VAL_IF_FALSE(m_connectionState == PulseConnectionState::PULSE_CONNECTION_LOADING ||
                            m_connectionState == PulseConnectionState::PULSE_CONNECTION_CONNECTED,
                        false);

    auto op = pa_context_get_card_info_by_name(m_context,
                                               name.toStdString().c_str(),
                                               &PulseContext::processCardInfo,
                                               this);
    return processPulseOperation(op);
}

bool PulseContext::loadSinkInfo(uint32_t index)
{
    pa_operation *op = NULL;

    RETURN_VAL_IF_FALSE(m_connectionState == PulseConnectionState::PULSE_CONNECTION_LOADING ||
                            m_connectionState == PulseConnectionState::PULSE_CONNECTION_CONNECTED,
                        false);

    // 如果未设置index，则获取所有的sink信息，否则获取指定的sink信息
    if (index != PA_INVALID_INDEX)
    {
        op = pa_context_get_sink_info_by_index(m_context, index, &PulseContext::processSinkInfo, this);
    }
    else
    {
        op = pa_context_get_sink_info_list(m_context, &PulseContext::processSinkInfo, this);
    }

    return processPulseOperation(op);
}

bool PulseContext::loadSinkInfoByName(const QString &name)
{
    RETURN_VAL_IF_FALSE(!name.isEmpty(), false);

    RETURN_VAL_IF_FALSE(m_connectionState == PulseConnectionState::PULSE_CONNECTION_LOADING ||
                            m_connectionState == PulseConnectionState::PULSE_CONNECTION_CONNECTED,
                        false);

    auto op = pa_context_get_sink_info_by_name(m_context,
                                               name.toStdString().c_str(),
                                               &PulseContext::processSinkInfo,
                                               this);

    return processPulseOperation(op);
}

bool PulseContext::loadSinkInputInfo(uint32_t index)
{
    pa_operation *op = NULL;

    RETURN_VAL_IF_FALSE(m_connectionState == PulseConnectionState::PULSE_CONNECTION_LOADING ||
                            m_connectionState == PulseConnectionState::PULSE_CONNECTION_CONNECTED,
                        false);

    // 如果未设置index，则获取所有的sink input信息，否则获取指定的sink input信息
    if (index != PA_INVALID_INDEX)
    {
        op = pa_context_get_sink_input_info(m_context,
                                            index,
                                            &PulseContext::processSinkInputInfo,
                                            this);
    }
    else
    {
        op = pa_context_get_sink_input_info_list(m_context, &PulseContext::processSinkInputInfo, this);
    }

    return processPulseOperation(op);
}

bool PulseContext::loadSourceInfo(uint32_t index)
{
    pa_operation *op = NULL;

    RETURN_VAL_IF_FALSE(m_connectionState == PulseConnectionState::PULSE_CONNECTION_LOADING ||
                            m_connectionState == PulseConnectionState::PULSE_CONNECTION_CONNECTED,
                        false);

    if (index != PA_INVALID_INDEX)
    {
        op = pa_context_get_source_info_by_index(m_context,
                                                 index,
                                                 &PulseContext::processSourceInfo,
                                                 this);
    }
    else
    {
        op = pa_context_get_source_info_list(m_context, &PulseContext::processSourceInfo, this);
    }

    return processPulseOperation(op);
}

bool PulseContext::loadSourceInfoByName(const QString &name)
{
    RETURN_VAL_IF_FALSE(!name.isEmpty(), false);

    RETURN_VAL_IF_FALSE(m_connectionState == PulseConnectionState::PULSE_CONNECTION_LOADING ||
                            m_connectionState == PulseConnectionState::PULSE_CONNECTION_CONNECTED,
                        false);

    auto op = pa_context_get_source_info_by_name(m_context,
                                                 name.toStdString().c_str(),
                                                 &PulseContext::processSourceInfo,
                                                 this);

    return processPulseOperation(op);
}

bool PulseContext::loadSourceOutputInfo(uint32_t index)
{
    pa_operation *op = NULL;

    RETURN_VAL_IF_FALSE(m_connectionState == PulseConnectionState::PULSE_CONNECTION_LOADING ||
                            m_connectionState == PulseConnectionState::PULSE_CONNECTION_CONNECTED,
                        false);

    if (index != PA_INVALID_INDEX)
    {
        op = pa_context_get_source_output_info(m_context,
                                               index,
                                               &PulseContext::processSourceOutputInfo,
                                               this);
    }
    else
    {
        op = pa_context_get_source_output_info_list(m_context, &PulseContext::processSourceOutputInfo, this);
    }

    return processPulseOperation(op);
}

bool PulseContext::setDefaultSink(const QString &name)
{
    RETURN_VAL_IF_FALSE(!name.isEmpty(), false);
    RETURN_VAL_IF_FALSE(m_connectionState == PulseConnectionState::PULSE_CONNECTION_CONNECTED, false);

    auto op = pa_context_set_default_sink(m_context, name.toStdString().c_str(), NULL, NULL);
    return processPulseOperation(op);
}

bool PulseContext::setDefaultSource(const QString &name)
{
    RETURN_VAL_IF_FALSE(!name.isEmpty(), false);
    RETURN_VAL_IF_FALSE(m_connectionState == PulseConnectionState::PULSE_CONNECTION_CONNECTED, false);

    auto op = pa_context_set_default_source(m_context, name.toStdString().c_str(), NULL, NULL);
    return processPulseOperation(op);
}

bool PulseContext::setCardProfile(const QString &card, const QString &profile)
{
    RETURN_VAL_IF_FALSE(!card.isEmpty(), false);
    RETURN_VAL_IF_FALSE(!profile.isEmpty(), false);
    RETURN_VAL_IF_FALSE(m_connectionState == PulseConnectionState::PULSE_CONNECTION_CONNECTED, false);

    auto op = pa_context_set_card_profile_by_name(m_context,
                                                  card.toStdString().c_str(),
                                                  profile.toStdString().c_str(), NULL, NULL);
    return processPulseOperation(op);
}

bool PulseContext::setSinkMute(uint32_t index, int32_t mute)
{
    RETURN_VAL_IF_FALSE(m_connectionState == PulseConnectionState::PULSE_CONNECTION_CONNECTED, false);

    auto op = pa_context_set_sink_mute_by_index(m_context, index, mute, NULL, NULL);
    return processPulseOperation(op);
}

bool PulseContext::setSinkVolume(uint32_t index, const pa_cvolume *volume)
{
    RETURN_VAL_IF_FALSE(volume != NULL, false);
    RETURN_VAL_IF_FALSE(m_connectionState == PulseConnectionState::PULSE_CONNECTION_CONNECTED, false);

    auto op = pa_context_set_sink_volume_by_index(m_context, index, volume, NULL, NULL);
    return processPulseOperation(op);
}

bool PulseContext::setSinkActivePort(uint32_t index, const QString &portName)
{
    RETURN_VAL_IF_FALSE(!portName.isEmpty(), false);
    RETURN_VAL_IF_FALSE(m_connectionState == PulseConnectionState::PULSE_CONNECTION_CONNECTED, false);

    auto op = pa_context_set_sink_port_by_index(m_context, index, portName.toStdString().c_str(), NULL, NULL);
    return processPulseOperation(op);
}

bool PulseContext::setSinkInputMute(uint32_t index, int32_t mute)
{
    KLOG_INFO(audio) << "Set sink input mute which index is " << index << " and mute is " << mute;

    RETURN_VAL_IF_FALSE(m_connectionState == PulseConnectionState::PULSE_CONNECTION_CONNECTED, false);

    auto op = pa_context_set_sink_input_mute(m_context, index, mute, NULL, NULL);
    return processPulseOperation(op);
}

bool PulseContext::setSinkInputVolume(uint32_t index, const pa_cvolume *volume)
{
    KLOG_INFO(audio) << "Set sink volume which index is " << index;

    RETURN_VAL_IF_FALSE(volume != NULL, false);
    RETURN_VAL_IF_FALSE(m_connectionState == PulseConnectionState::PULSE_CONNECTION_CONNECTED, false);

    auto op = pa_context_set_sink_input_volume(m_context, index, volume, NULL, NULL);
    return processPulseOperation(op);
}

bool PulseContext::setSourceMute(uint32_t index, int32_t mute)
{
    RETURN_VAL_IF_FALSE(m_connectionState == PulseConnectionState::PULSE_CONNECTION_CONNECTED, false);

    auto op = pa_context_set_source_mute_by_index(m_context, index, mute, NULL, NULL);
    return processPulseOperation(op);
}

bool PulseContext::setSourceVolume(uint32_t index, const pa_cvolume *volume)
{
    RETURN_VAL_IF_FALSE(volume != NULL, false);
    RETURN_VAL_IF_FALSE(m_connectionState == PulseConnectionState::PULSE_CONNECTION_CONNECTED, false);

    auto op = pa_context_set_source_volume_by_index(m_context, index, volume, NULL, NULL);
    return processPulseOperation(op);
}

bool PulseContext::setSourceActivePort(uint32_t index, const QString &portName)
{
    RETURN_VAL_IF_FALSE(!portName.isEmpty(), false);
    RETURN_VAL_IF_FALSE(m_connectionState == PulseConnectionState::PULSE_CONNECTION_CONNECTED, false);

    auto op = pa_context_set_source_port_by_index(m_context, index, portName.toStdString().c_str(), NULL, NULL);
    return processPulseOperation(op);
}

bool PulseContext::setSourceOutputMute(uint32_t index, int32_t mute)
{
    RETURN_VAL_IF_FALSE(m_connectionState == PulseConnectionState::PULSE_CONNECTION_CONNECTED, false);

    auto op = pa_context_set_source_output_mute(m_context, index, mute, NULL, NULL);
    return processPulseOperation(op);
}

bool PulseContext::setSourceOutputVolume(uint32_t index, const pa_cvolume *volume)
{
    RETURN_VAL_IF_FALSE(volume != NULL, false);
    RETURN_VAL_IF_FALSE(m_connectionState == PulseConnectionState::PULSE_CONNECTION_CONNECTED, false);

    auto op = pa_context_set_source_output_volume(m_context, index, volume, NULL, NULL);
    return processPulseOperation(op);
}

bool PulseContext::suspendSink(uint32_t index, bool suspend)
{
    RETURN_VAL_IF_FALSE(m_connectionState == PulseConnectionState::PULSE_CONNECTION_CONNECTED, false);

    auto op = pa_context_suspend_sink_by_index(m_context, index, (int)suspend, NULL, NULL);
    return processPulseOperation(op);
}

bool PulseContext::suspendSource(uint32_t index, bool suspend)
{
    RETURN_VAL_IF_FALSE(m_connectionState == PulseConnectionState::PULSE_CONNECTION_CONNECTED, false);

    auto op = pa_context_suspend_source_by_index(m_context, index, (int)suspend, NULL, NULL);
    return processPulseOperation(op);
}

bool PulseContext::moveSinkInput(uint32_t index, uint32_t sink_index)
{
    RETURN_VAL_IF_FALSE(m_connectionState == PulseConnectionState::PULSE_CONNECTION_CONNECTED, false);

    auto op = pa_context_move_sink_input_by_index(m_context, index, sink_index, NULL, NULL);
    return processPulseOperation(op);
}

bool PulseContext::moveSourceOutput(uint32_t index, uint32_t source_index)
{
    RETURN_VAL_IF_FALSE(m_connectionState == PulseConnectionState::PULSE_CONNECTION_CONNECTED, false);

    auto op = pa_context_move_source_output_by_index(m_context, index, source_index, NULL, NULL);
    return processPulseOperation(op);
}

bool PulseContext::killSinkInput(uint32_t index)
{
    RETURN_VAL_IF_FALSE(m_connectionState == PulseConnectionState::PULSE_CONNECTION_CONNECTED, false);

    auto op = pa_context_kill_sink_input(m_context, index, NULL, NULL);
    return processPulseOperation(op);
}

bool PulseContext::killSourceOutput(uint32_t index)
{
    RETURN_VAL_IF_FALSE(m_connectionState == PulseConnectionState::PULSE_CONNECTION_CONNECTED, false);

    auto op = pa_context_kill_source_output(m_context, index, NULL, NULL);
    return processPulseOperation(op);
}

// void PulseContext::iteratePulseEvent()
// {
//     int retval = 0;

//     while (pa_mainloop_iterate(m_mainLoop, 1, &retval) > 0)
//         ;

//     if (retval < 0)
//     {
//         KLOG_ERROR(audio) << "pa_mainloop_iterate failed with error code: " << retval;
//     }
// }

void PulseContext::setConnectionState(PulseConnectionState new_state)
{
    RETURN_IF_TRUE(m_connectionState == new_state);

    m_connectionState = new_state;
    Q_EMIT connectionStateChanged(m_connectionState);
}

bool PulseContext::loadLists()
{
    QList<pa_operation *> opList;
    pa_operation *op = NULL;

    // 之前的加载请求还未完成，不允许重复加载
    if (m_requestCount > 0)
    {
        KLOG_WARNING(audio) << "The previous request hasn't finished. The remaing request count is " << m_requestCount;
        return false;
    }

#define INSERT_OP_TO_LIST(op)  \
    {                          \
        if (op == NULL)        \
        {                      \
            break;             \
        }                      \
        opList.push_front(op); \
        ++m_requestCount;      \
    }

    do
    {
        op = pa_context_get_card_info_list(m_context, &PulseContext::processCardInfo, this);
        INSERT_OP_TO_LIST(op);

        op = pa_context_get_sink_info_list(m_context, &PulseContext::processSinkInfo, this);
        INSERT_OP_TO_LIST(op);

        op = pa_context_get_sink_input_info_list(m_context, &PulseContext::processSinkInputInfo, this);
        INSERT_OP_TO_LIST(op);

        op = pa_context_get_source_info_list(m_context, &PulseContext::processSourceInfo, this);
        INSERT_OP_TO_LIST(op);

        op = pa_context_get_source_output_info_list(m_context, &PulseContext::processSourceOutputInfo, this);
        INSERT_OP_TO_LIST(op);

        // 该操作不一定支持
        // op = pa_ext_stream_restore_read(context_, &PulseContext::on_pulse_ext_stream_restore_cb, this);
        // if (op != NULL)
        // {
        //     ops = g_slist_prepend(ops, op);
        //     ++request_count_;
        // }

        KLOG_DEBUG(audio) << "Request count is " << m_requestCount;

        for (auto op : opList)
        {
            pa_operation_unref(op);
        }

        return true;
    } while (0);

    for (auto op : opList)
    {
        pa_operation_cancel(op);
        pa_operation_unref(op);
    }

    return false;
}

bool PulseContext::processPulseOperation(pa_operation *op)
{
    if (!op)
    {
        KLOG_WARNING(audio) << "PulseAudio operation failed: " << pa_strerror(pa_context_errno(m_context));
        return false;
    }
    pa_operation_unref(op);
    return true;
}

bool PulseContext::loadListFinished()
{
    KLOG_DEBUG(audio) << "Request count is " << m_requestCount;

    // 如果小于等于0说明代码逻辑存在错误
    if (m_requestCount <= 0)
    {
        KLOG_WARNING(audio) << "The request count is invalid. The request count is " << m_requestCount;
        m_requestCount = 0;
        return false;
    }

    --m_requestCount;

    // 所有请求的回调都已经完成，最后一步加载服务器信息
    if (m_requestCount == 0)
    {
        if (!loadServerInfo())
        {
            disconnect();
            return false;
        }
    }

    return true;
}

void PulseContext::processStateChanged(pa_context *context, void *userdata)
{
    PulseContext *self = static_cast<PulseContext *>(userdata);
    auto state = pa_context_get_state(self->m_context);

    if (state == PA_CONTEXT_READY)
    {
        if (self->m_connectionState == PULSE_CONNECTION_LOADING || self->m_connectionState == PULSE_CONNECTION_CONNECTED)
        {
            KLOG_WARNING(audio) << "The connection state is mismatch with real state.";
            return;
        }

        // 连接已准备，设置订阅通知的回调函数
        pa_context_set_subscribe_callback(self->m_context, &PulseContext::processSubscribeEvent, self);
        // pa_ext_stream_restore_set_subscribe_cb(self->context, pulse_restore_subscribe_cb, self);
        // auto op = pa_ext_stream_restore_subscribe(self->context, TRUE, NULL, NULL);
        KLOG_DEBUG(audio) << "Pulse state change, state is " << state;
        /* Keep going if this operation fails */
        // process_pulse_operation(connection, op);

        // 订阅相关的通知信息
        auto op = pa_context_subscribe(self->m_context,
                                       pa_subscription_mask_t(PA_SUBSCRIPTION_MASK_SERVER | PA_SUBSCRIPTION_MASK_CARD | PA_SUBSCRIPTION_MASK_SINK |
                                                              PA_SUBSCRIPTION_MASK_SOURCE | PA_SUBSCRIPTION_MASK_SINK_INPUT | PA_SUBSCRIPTION_MASK_SOURCE_OUTPUT),
                                       NULL, NULL);

        if (self->processPulseOperation(op))
        {
            self->setConnectionState(PulseConnectionState::PULSE_CONNECTION_LOADING);

            if (!self->loadLists())
            {
                state = PA_CONTEXT_FAILED;
            }
        }
        else
        {
            state = PA_CONTEXT_FAILED;
        }
    }

    switch (state)
    {
    case PA_CONTEXT_TERMINATED:
    case PA_CONTEXT_FAILED:
        self->disconnect();
        break;
    case PA_CONTEXT_CONNECTING:
        self->setConnectionState(PulseConnectionState::PULSE_CONNECTION_CONNECTING);
        break;
    case PA_CONTEXT_AUTHORIZING:
    case PA_CONTEXT_SETTING_NAME:
        self->setConnectionState(PulseConnectionState::PULSE_CONNECTION_AUTHORIZING);
        break;
    default:
        break;
    }
    return;
}

static QString event2facility(int32_t event)
{
    auto facility = (event & PA_SUBSCRIPTION_EVENT_FACILITY_MASK);
    switch (facility)
    {
    case PA_SUBSCRIPTION_EVENT_SERVER:
        return "Server";
    case PA_SUBSCRIPTION_EVENT_CARD:
        return "Card";
    case PA_SUBSCRIPTION_EVENT_SINK:
        return "Sink";
    case PA_SUBSCRIPTION_EVENT_SINK_INPUT:
        return "SinkInput";
    case PA_SUBSCRIPTION_EVENT_SOURCE:
        return "Source";
    case PA_SUBSCRIPTION_EVENT_SOURCE_OUTPUT:
        return "SourceOutput";
    default:
        return "Other";
    }
    return "Other";
}

static QString event2type(int32_t event)
{
    auto type = (event & PA_SUBSCRIPTION_EVENT_TYPE_MASK);
    switch (type)
    {
    case PA_SUBSCRIPTION_EVENT_NEW:
        return "New";
    case PA_SUBSCRIPTION_EVENT_CHANGE:
        return "Change";
    case PA_SUBSCRIPTION_EVENT_REMOVE:
        return "Remove";
    default:
        return "Other";
    }
    return "Other";
}

void PulseContext::processSubscribeEvent(pa_context *context,
                                         pa_subscription_event_type_t event_type,
                                         uint32_t idx,
                                         void *userdata)
{
    PulseContext *self = static_cast<PulseContext *>(userdata);

    KLOG_DEBUG(audio) << "Receive subscribe event. "
                      << "The facility is" << event2facility(event_type)
                      << ", type is" << event2type(event_type)
                      << ", idx is " << idx;

    auto type = (event_type & PA_SUBSCRIPTION_EVENT_TYPE_MASK);

    switch (event_type & PA_SUBSCRIPTION_EVENT_FACILITY_MASK)
    {
    case PA_SUBSCRIPTION_EVENT_SERVER:
        self->loadServerInfo();
        break;
    case PA_SUBSCRIPTION_EVENT_CARD:
        if (type == PA_SUBSCRIPTION_EVENT_REMOVE)
        {
            Q_EMIT self->cardInfoRemoved(idx);
        }
        else
        {
            self->loadCardInfo(idx);
        }
        break;
    case PA_SUBSCRIPTION_EVENT_SINK:
        if (type == PA_SUBSCRIPTION_EVENT_REMOVE)
        {
            Q_EMIT self->sinkInfoRemoved(idx);
        }
        else
        {
            self->loadSinkInfo(idx);
        }
        break;
    case PA_SUBSCRIPTION_EVENT_SINK_INPUT:
        if (type == PA_SUBSCRIPTION_EVENT_REMOVE)
        {
            Q_EMIT self->sinkInputInfoRemoved(idx);
        }
        else
        {
            self->loadSinkInputInfo(idx);
        }
        break;
    case PA_SUBSCRIPTION_EVENT_SOURCE:
        if (type == PA_SUBSCRIPTION_EVENT_REMOVE)
        {
            Q_EMIT self->sourceInfoRemoved(idx);
        }
        else
        {
            self->loadSourceInfo(idx);
        }
        break;
    case PA_SUBSCRIPTION_EVENT_SOURCE_OUTPUT:
        if (type == PA_SUBSCRIPTION_EVENT_REMOVE)
        {
            Q_EMIT self->sourceOutputInfoRemoved(idx);
        }
        else
        {
            self->loadSourceOutputInfo(idx);
        }
        break;
    default:
        break;
    }
}

void PulseContext::processServerInfo(pa_context *context, const pa_server_info *serverInfo, void *userdata)
{
    PulseContext *self = static_cast<PulseContext *>(userdata);

    if (self == NULL || self->m_context != context)
    {
        return;
    }

    Q_EMIT self->serverInfoChanged(serverInfo);
    // 服务器信息的回调函数是最后一个回调函数
    if (self->m_connectionState == PulseConnectionState::PULSE_CONNECTION_LOADING)
    {
        self->setConnectionState(PulseConnectionState::PULSE_CONNECTION_CONNECTED);
    }
}

// 只有在加载状态时才需要调用load_list_finished，因为服务器信息(server info)需要等待sink/source等请求完毕后才进行加载
#define ON_PULSE_INFO_CB(func, prop, signal)                                                                           \
    void PulseContext::process##func##Info(pa_context *context, const pa_##prop##_info *info, int eol, void *userdata) \
    {                                                                                                                  \
        PulseContext *self = static_cast<PulseContext *>(userdata);                                                    \
        RETURN_IF_FALSE(self != NULL && self->m_context == context);                                                   \
                                                                                                                       \
        if (eol)                                                                                                       \
        {                                                                                                              \
            if (self->m_connectionState == PulseConnectionState::PULSE_CONNECTION_LOADING)                             \
            {                                                                                                          \
                self->loadListFinished();                                                                              \
            }                                                                                                          \
            return;                                                                                                    \
        }                                                                                                              \
        Q_EMIT self->signal##InfoChanged(info);                                                                        \
    }

ON_PULSE_INFO_CB(Card, card, card)
ON_PULSE_INFO_CB(Sink, sink, sink)
ON_PULSE_INFO_CB(SinkInput, sink_input, sinkInput)
ON_PULSE_INFO_CB(Source, source, source)
ON_PULSE_INFO_CB(SourceOutput, source_output, sourceOutput)

}  // namespace Kiran
