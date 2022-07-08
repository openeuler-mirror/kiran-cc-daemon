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

#include "plugins/audio/pulse/pulse-context.h"
#include "audio-i.h"
#include "config.h"

namespace Kiran
{
PulseContext::PulseContext() : main_loop_(NULL),
                               props_(NULL),
                               context_(NULL),
                               connection_state_(PulseConnectionState::PULSE_CONNECTION_DISCONNECTED),
                               request_count_(0)
{
    this->props_ = pa_proplist_new();
    pa_proplist_sets(this->props_, PA_PROP_APPLICATION_NAME, this->get_default_app_name().c_str());
    pa_proplist_sets(this->props_, PA_PROP_APPLICATION_ID, AUDIO_DBUS_NAME);
    pa_proplist_sets(this->props_, PA_PROP_APPLICATION_VERSION, PROJECT_VERSION);

    this->main_loop_ = pa_glib_mainloop_new(g_main_context_get_thread_default());
    if (!this->main_loop_)
    {
        KLOG_WARNING("Failed to create PulseAudio main loop");
    }
}

PulseContext::~PulseContext()
{
    if (this->context_ != NULL)
    {
        pa_context_unref(this->context_);
    }

    if (this->props_)
    {
        pa_proplist_free(this->props_);
    }

    if (this->main_loop_)
    {
        pa_glib_mainloop_free(this->main_loop_);
    }
}

bool PulseContext::connect(bool wait_for_daemon)
{
    KLOG_PROFILE("wait for deamon: %d.", wait_for_daemon);

    RETURN_VAL_IF_FALSE(this->main_loop_ != NULL, false);

    // 如果已经连接或者正在连接，则直接返回
    RETURN_VAL_IF_TRUE(this->connection_state_ != PulseConnectionState::PULSE_CONNECTION_DISCONNECTED, true);

    auto mainloop = pa_glib_mainloop_get_api(this->main_loop_);
    this->context_ = pa_context_new_with_proplist(mainloop, NULL, this->props_);
    if (!this->context_)
    {
        KLOG_WARNING("Failed to create PulseAudio context");
        return false;
    }

    // 连接服务器为异步操作，需要设置回调函数监听连接的状态变化
    pa_context_set_state_callback(this->context_, &PulseContext::on_pulse_state_cb, this);

    pa_context_flags_t flags = wait_for_daemon ? PA_CONTEXT_NOFAIL : PA_CONTEXT_NOFLAGS;

    if (pa_context_connect(this->context_, NULL, flags, NULL) == 0)
    {
        this->set_connection_state(PulseConnectionState::PULSE_CONNECTION_CONNECTING);
        return true;
    }
    else
    {
        pa_context_unref(this->context_);
        this->context_ = NULL;
        return false;
    }
}

void PulseContext::disconnect()
{
    RETURN_IF_TRUE(this->connection_state_ == PulseConnectionState::PULSE_CONNECTION_DISCONNECTED);

    if (this->context_ != NULL)
    {
        pa_context_unref(this->context_);
        this->context_ = NULL;
    }
    this->request_count_ = 0;

    // connection->priv->ext_streams_loading = FALSE;
    // connection->priv->ext_streams_dirty = FALSE;

    this->set_connection_state(PulseConnectionState::PULSE_CONNECTION_DISCONNECTED);
}

bool PulseContext::load_server_info()
{
    RETURN_VAL_IF_FALSE(this->connection_state_ == PulseConnectionState::PULSE_CONNECTION_LOADING ||
                            this->connection_state_ == PulseConnectionState::PULSE_CONNECTION_CONNECTED,
                        false);

    auto op = pa_context_get_server_info(this->context_, &PulseContext::on_pulse_server_info_cb, this);
    return this->process_pulse_operation(op);
}

bool PulseContext::load_card_info(uint32_t index)
{
    pa_operation *op = NULL;

    RETURN_VAL_IF_FALSE(this->connection_state_ == PulseConnectionState::PULSE_CONNECTION_LOADING ||
                            this->connection_state_ == PulseConnectionState::PULSE_CONNECTION_CONNECTED,
                        false);

    // 如果未设置index，则获取所有的card信息，否则获取指定的card信息
    if (index != PA_INVALID_INDEX)
    {
        op = pa_context_get_card_info_by_index(this->context_, index, &PulseContext::on_pulse_card_info_cb, this);
    }
    else
    {
        op = pa_context_get_card_info_list(this->context_, &PulseContext::on_pulse_card_info_cb, this);
    }

    return this->process_pulse_operation(op);
}

bool PulseContext::load_card_info_by_name(const std::string &name)
{
    KLOG_DEBUG("Load card info: %s.", name.c_str());

    RETURN_VAL_IF_FALSE(!name.empty(), false);

    RETURN_VAL_IF_FALSE(this->connection_state_ == PulseConnectionState::PULSE_CONNECTION_LOADING ||
                            this->connection_state_ == PulseConnectionState::PULSE_CONNECTION_CONNECTED,
                        false);

    auto op = pa_context_get_card_info_by_name(this->context_,
                                               name.c_str(),
                                               &PulseContext::on_pulse_card_info_cb,
                                               this);
    return this->process_pulse_operation(op);
}

bool PulseContext::load_sink_info(uint32_t index)
{
    pa_operation *op = NULL;

    RETURN_VAL_IF_FALSE(this->connection_state_ == PulseConnectionState::PULSE_CONNECTION_LOADING ||
                            this->connection_state_ == PulseConnectionState::PULSE_CONNECTION_CONNECTED,
                        false);

    // 如果未设置index，则获取所有的sink信息，否则获取指定的sink信息
    if (index != PA_INVALID_INDEX)
    {
        op = pa_context_get_sink_info_by_index(this->context_, index, &PulseContext::on_pulse_sink_info_cb, this);
    }
    else
    {
        op = pa_context_get_sink_info_list(this->context_, &PulseContext::on_pulse_sink_info_cb, this);
    }

    return this->process_pulse_operation(op);
}

bool PulseContext::load_sink_info_by_name(const std::string &name)
{
    KLOG_PROFILE("load sink info by name: %s.", name.c_str());

    RETURN_VAL_IF_FALSE(!name.empty(), false);

    RETURN_VAL_IF_FALSE(this->connection_state_ == PulseConnectionState::PULSE_CONNECTION_LOADING ||
                            this->connection_state_ == PulseConnectionState::PULSE_CONNECTION_CONNECTED,
                        false);

    auto op = pa_context_get_sink_info_by_name(this->context_,
                                               name.c_str(),
                                               &PulseContext::on_pulse_sink_info_cb,
                                               this);

    return this->process_pulse_operation(op);
}

bool PulseContext::load_sink_input_info(uint32_t index)
{
    pa_operation *op = NULL;

    RETURN_VAL_IF_FALSE(this->connection_state_ == PulseConnectionState::PULSE_CONNECTION_LOADING ||
                            this->connection_state_ == PulseConnectionState::PULSE_CONNECTION_CONNECTED,
                        false);

    // 如果未设置index，则获取所有的sink input信息，否则获取指定的sink input信息
    if (index != PA_INVALID_INDEX)
    {
        op = pa_context_get_sink_input_info(this->context_,
                                            index,
                                            &PulseContext::on_pulse_sink_input_info_cb,
                                            this);
    }
    else
    {
        op = pa_context_get_sink_input_info_list(this->context_, &PulseContext::on_pulse_sink_input_info_cb, this);
    }

    return this->process_pulse_operation(op);
}

bool PulseContext::load_source_info(uint32_t index)
{
    pa_operation *op = NULL;

    RETURN_VAL_IF_FALSE(this->connection_state_ == PulseConnectionState::PULSE_CONNECTION_LOADING ||
                            this->connection_state_ == PulseConnectionState::PULSE_CONNECTION_CONNECTED,
                        false);

    if (index != PA_INVALID_INDEX)
    {
        op = pa_context_get_source_info_by_index(this->context_,
                                                 index,
                                                 &PulseContext::on_pulse_source_info_cb,
                                                 this);
    }
    else
    {
        op = pa_context_get_source_info_list(this->context_, &PulseContext::on_pulse_source_info_cb, this);
    }

    return this->process_pulse_operation(op);
}

bool PulseContext::load_source_info_by_name(const std::string &name)
{
    KLOG_PROFILE("load source info by name: %d.", name.c_str());

    RETURN_VAL_IF_FALSE(!name.empty(), false);

    RETURN_VAL_IF_FALSE(this->connection_state_ == PulseConnectionState::PULSE_CONNECTION_LOADING ||
                            this->connection_state_ == PulseConnectionState::PULSE_CONNECTION_CONNECTED,
                        false);

    auto op = pa_context_get_source_info_by_name(this->context_,
                                                 name.c_str(),
                                                 &PulseContext::on_pulse_source_info_cb,
                                                 this);

    return this->process_pulse_operation(op);
}

bool PulseContext::load_source_output_info(uint32_t index)
{
    pa_operation *op = NULL;

    RETURN_VAL_IF_FALSE(this->connection_state_ == PulseConnectionState::PULSE_CONNECTION_LOADING ||
                            this->connection_state_ == PulseConnectionState::PULSE_CONNECTION_CONNECTED,
                        false);

    if (index != PA_INVALID_INDEX)
    {
        op = pa_context_get_source_output_info(this->context_,
                                               index,
                                               &PulseContext::on_pulse_source_output_info_cb,
                                               this);
    }
    else
    {
        op = pa_context_get_source_output_info_list(this->context_, &PulseContext::on_pulse_source_output_info_cb, this);
    }

    return this->process_pulse_operation(op);
}

bool PulseContext::set_default_sink(const std::string &name)
{
    KLOG_PROFILE("sink name: %s.", name.c_str());

    RETURN_VAL_IF_FALSE(!name.empty(), false);
    RETURN_VAL_IF_FALSE(this->connection_state_ == PulseConnectionState::PULSE_CONNECTION_CONNECTED, false);

    auto op = pa_context_set_default_sink(this->context_, name.c_str(), NULL, NULL);
    return this->process_pulse_operation(op);
}

bool PulseContext::set_default_source(const std::string &name)
{
    KLOG_PROFILE("source name: %s.", name.c_str());

    RETURN_VAL_IF_FALSE(!name.empty(), false);
    RETURN_VAL_IF_FALSE(this->connection_state_ == PulseConnectionState::PULSE_CONNECTION_CONNECTED, false);

    auto op = pa_context_set_default_source(this->context_, name.c_str(), NULL, NULL);
    return this->process_pulse_operation(op);
}

bool PulseContext::set_card_profile(const std::string &card, const std::string &profile)
{
    RETURN_VAL_IF_FALSE(!card.empty(), false);
    RETURN_VAL_IF_FALSE(!profile.empty(), false);
    RETURN_VAL_IF_FALSE(this->connection_state_ == PulseConnectionState::PULSE_CONNECTION_CONNECTED, false);

    auto op = pa_context_set_card_profile_by_name(this->context_, card.c_str(), profile.c_str(), NULL, NULL);
    return this->process_pulse_operation(op);
}

bool PulseContext::set_sink_mute(uint32_t index, int32_t mute)
{
    RETURN_VAL_IF_FALSE(this->connection_state_ == PulseConnectionState::PULSE_CONNECTION_CONNECTED, false);

    auto op = pa_context_set_sink_mute_by_index(this->context_, index, mute, NULL, NULL);
    return this->process_pulse_operation(op);
}

bool PulseContext::set_sink_volume(uint32_t index, const pa_cvolume *volume)
{
    RETURN_VAL_IF_FALSE(volume != NULL, false);
    RETURN_VAL_IF_FALSE(this->connection_state_ == PulseConnectionState::PULSE_CONNECTION_CONNECTED, false);

    auto op = pa_context_set_sink_volume_by_index(this->context_, index, volume, NULL, NULL);
    return this->process_pulse_operation(op);
}

bool PulseContext::set_sink_active_port(uint32_t index, const std::string &port_name)
{
    RETURN_VAL_IF_FALSE(!port_name.empty(), false);
    RETURN_VAL_IF_FALSE(this->connection_state_ == PulseConnectionState::PULSE_CONNECTION_CONNECTED, false);

    auto op = pa_context_set_sink_port_by_index(this->context_, index, port_name.c_str(), NULL, NULL);
    return this->process_pulse_operation(op);
}

bool PulseContext::set_sink_input_mute(uint32_t index, int32_t mute)
{
    KLOG_PROFILE("sink index: %d, mute: %d.", index, mute);

    RETURN_VAL_IF_FALSE(this->connection_state_ == PulseConnectionState::PULSE_CONNECTION_CONNECTED, false);

    auto op = pa_context_set_sink_input_mute(this->context_, index, mute, NULL, NULL);
    return this->process_pulse_operation(op);
}

bool PulseContext::set_sink_input_volume(uint32_t index, const pa_cvolume *volume)
{
    KLOG_PROFILE("sink index: %d.", index);

    RETURN_VAL_IF_FALSE(volume != NULL, false);
    RETURN_VAL_IF_FALSE(this->connection_state_ == PulseConnectionState::PULSE_CONNECTION_CONNECTED, false);

    auto op = pa_context_set_sink_input_volume(this->context_, index, volume, NULL, NULL);
    return this->process_pulse_operation(op);
}

bool PulseContext::set_source_mute(uint32_t index, int32_t mute)
{
    RETURN_VAL_IF_FALSE(this->connection_state_ == PulseConnectionState::PULSE_CONNECTION_CONNECTED, false);

    auto op = pa_context_set_source_mute_by_index(this->context_, index, mute, NULL, NULL);
    return this->process_pulse_operation(op);
}

bool PulseContext::set_source_volume(uint32_t index, const pa_cvolume *volume)
{
    RETURN_VAL_IF_FALSE(volume != NULL, false);
    RETURN_VAL_IF_FALSE(this->connection_state_ == PulseConnectionState::PULSE_CONNECTION_CONNECTED, false);

    auto op = pa_context_set_source_volume_by_index(this->context_, index, volume, NULL, NULL);
    return this->process_pulse_operation(op);
}

bool PulseContext::set_source_active_port(uint32_t index, const std::string &port_name)
{
    RETURN_VAL_IF_FALSE(!port_name.empty(), false);
    RETURN_VAL_IF_FALSE(this->connection_state_ == PulseConnectionState::PULSE_CONNECTION_CONNECTED, false);

    auto op = pa_context_set_source_port_by_index(this->context_, index, port_name.c_str(), NULL, NULL);
    return this->process_pulse_operation(op);
}

bool PulseContext::set_source_output_mute(uint32_t index, int32_t mute)
{
    RETURN_VAL_IF_FALSE(this->connection_state_ == PulseConnectionState::PULSE_CONNECTION_CONNECTED, false);

    auto op = pa_context_set_source_output_mute(this->context_, index, mute, NULL, NULL);
    return this->process_pulse_operation(op);
}

bool PulseContext::set_source_output_volume(uint32_t index, const pa_cvolume *volume)
{
    RETURN_VAL_IF_FALSE(volume != NULL, false);
    RETURN_VAL_IF_FALSE(this->connection_state_ == PulseConnectionState::PULSE_CONNECTION_CONNECTED, false);

    auto op = pa_context_set_source_output_volume(this->context_, index, volume, NULL, NULL);
    return this->process_pulse_operation(op);
}

bool PulseContext::suspend_sink(uint32_t index, bool suspend)
{
    RETURN_VAL_IF_FALSE(this->connection_state_ == PulseConnectionState::PULSE_CONNECTION_CONNECTED, false);

    auto op = pa_context_suspend_sink_by_index(this->context_, index, (int)suspend, NULL, NULL);
    return this->process_pulse_operation(op);
}

bool PulseContext::suspend_source(uint32_t index, bool suspend)
{
    RETURN_VAL_IF_FALSE(this->connection_state_ == PulseConnectionState::PULSE_CONNECTION_CONNECTED, false);

    auto op = pa_context_suspend_source_by_index(this->context_, index, (int)suspend, NULL, NULL);
    return this->process_pulse_operation(op);
}

bool PulseContext::move_sink_input(uint32_t index, uint32_t sink_index)
{
    RETURN_VAL_IF_FALSE(this->connection_state_ == PulseConnectionState::PULSE_CONNECTION_CONNECTED, false);

    auto op = pa_context_move_sink_input_by_index(this->context_, index, sink_index, NULL, NULL);
    return this->process_pulse_operation(op);
}

bool PulseContext::move_source_output(uint32_t index, uint32_t source_index)
{
    RETURN_VAL_IF_FALSE(this->connection_state_ == PulseConnectionState::PULSE_CONNECTION_CONNECTED, false);

    auto op = pa_context_move_source_output_by_index(this->context_, index, source_index, NULL, NULL);
    return this->process_pulse_operation(op);
}

bool PulseContext::kill_sink_input(uint32_t index)
{
    RETURN_VAL_IF_FALSE(this->connection_state_ == PulseConnectionState::PULSE_CONNECTION_CONNECTED, false);

    auto op = pa_context_kill_sink_input(this->context_, index, NULL, NULL);
    return this->process_pulse_operation(op);
}

bool PulseContext::kill_source_output(uint32_t index)
{
    RETURN_VAL_IF_FALSE(this->connection_state_ == PulseConnectionState::PULSE_CONNECTION_CONNECTED, false);

    auto op = pa_context_kill_source_output(this->context_, index, NULL, NULL);
    return this->process_pulse_operation(op);
}

void PulseContext::set_connection_state(PulseConnectionState new_state)
{
    RETURN_IF_TRUE(this->connection_state_ == new_state);

    this->connection_state_ = new_state;
    this->connection_state_changed_.emit(this->connection_state_);
    return;
}

bool PulseContext::load_lists()
{
    KLOG_PROFILE("");

    GSList *ops = NULL;
    pa_operation *op = NULL;

    // 之前的加载请求还未完成，不允许重复加载
    if (this->request_count_ > 0)
    {
        KLOG_WARNING("The previous request hasn't finished. The remaing request count: %d.", this->request_count_);
        return false;
    }

#define INSERT_OP_TO_LIST(op)           \
    {                                   \
        if (op == NULL)                 \
        {                               \
            break;                      \
        }                               \
        ops = g_slist_prepend(ops, op); \
        ++this->request_count_;         \
    }

    do
    {
        op = pa_context_get_card_info_list(this->context_, &PulseContext::on_pulse_card_info_cb, this);
        INSERT_OP_TO_LIST(op);

        op = pa_context_get_sink_info_list(this->context_, &PulseContext::on_pulse_sink_info_cb, this);
        INSERT_OP_TO_LIST(op);

        op = pa_context_get_sink_input_info_list(this->context_, &PulseContext::on_pulse_sink_input_info_cb, this);
        INSERT_OP_TO_LIST(op);

        op = pa_context_get_source_info_list(this->context_, &PulseContext::on_pulse_source_info_cb, this);
        INSERT_OP_TO_LIST(op);

        op = pa_context_get_source_output_info_list(this->context_, &PulseContext::on_pulse_source_output_info_cb, this);
        INSERT_OP_TO_LIST(op);

        // 该操作不一定支持
        // op = pa_ext_stream_restore_read(this->context_, &PulseContext::on_pulse_ext_stream_restore_cb, this);
        // if (op != NULL)
        // {
        //     ops = g_slist_prepend(ops, op);
        //     ++this->request_count_;
        // }

        KLOG_DEBUG("Request count: %d.", this->request_count_);

        g_slist_foreach(ops, (GFunc)pa_operation_unref, NULL);
        g_slist_free(ops);
        return true;
    } while (0);

    g_slist_foreach(ops, (GFunc)pa_operation_cancel, NULL);
    g_slist_foreach(ops, (GFunc)pa_operation_unref, NULL);
    g_slist_free(ops);
    return false;
}

bool PulseContext::process_pulse_operation(pa_operation *op)
{
    if (!op)
    {
        KLOG_WARNING("PulseAudio operation failed: %s", pa_strerror(pa_context_errno(this->context_)));
        return false;
    }
    pa_operation_unref(op);
    return true;
}

bool PulseContext::load_list_finished()
{
    KLOG_PROFILE("Request count: %d.", this->request_count_);

    // 如果小于等于0说明代码逻辑存在错误
    if (this->request_count_ <= 0)
    {
        KLOG_WARNING("The request count is invalid. The request count: %d.", this->request_count_);
        this->request_count_ = 0;
        return false;
    }

    --this->request_count_;

    // 所有请求的回调都已经完成，最后一步加载服务器信息
    if (this->request_count_ == 0)
    {
        if (!this->load_server_info())
        {
            this->disconnect();
            return false;
        }
    }

    return true;
}

void PulseContext::on_pulse_state_cb(pa_context *context, void *userdata)
{
    PulseContext *self = (PulseContext *)(userdata);
    auto state = pa_context_get_state(self->context_);

    if (state == PA_CONTEXT_READY)
    {
        if (self->connection_state_ == PULSE_CONNECTION_LOADING || self->connection_state_ == PULSE_CONNECTION_CONNECTED)
        {
            KLOG_WARNING("The connection state is mismatch with real state.");
            return;
        }

        // 连接已准备，设置订阅通知的回调函数
        pa_context_set_subscribe_callback(self->context_, &PulseContext::on_pulse_subscribe_cb, self);
        // pa_ext_stream_restore_set_subscribe_cb(self->context, pulse_restore_subscribe_cb, self);
        // auto op = pa_ext_stream_restore_subscribe(self->context, TRUE, NULL, NULL);

        /* Keep going if this operation fails */
        // process_pulse_operation(connection, op);

        // 订阅相关的通知信息
        auto op = pa_context_subscribe(self->context_,
                                       pa_subscription_mask_t(PA_SUBSCRIPTION_MASK_SERVER | PA_SUBSCRIPTION_MASK_CARD | PA_SUBSCRIPTION_MASK_SINK |
                                                              PA_SUBSCRIPTION_MASK_SOURCE | PA_SUBSCRIPTION_MASK_SINK_INPUT | PA_SUBSCRIPTION_MASK_SOURCE_OUTPUT),
                                       NULL, NULL);

        if (self->process_pulse_operation(op))
        {
            self->set_connection_state(PulseConnectionState::PULSE_CONNECTION_LOADING);

            if (!self->load_lists())
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
        self->set_connection_state(PulseConnectionState::PULSE_CONNECTION_CONNECTING);
        break;
    case PA_CONTEXT_AUTHORIZING:
    case PA_CONTEXT_SETTING_NAME:
        self->set_connection_state(PulseConnectionState::PULSE_CONNECTION_AUTHORIZING);
        break;
    default:
        break;
    }
    return;
}

static std::string event2facility(int32_t event)
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

static std::string event2type(int32_t event)
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

void PulseContext::on_pulse_subscribe_cb(pa_context *context,
                                         pa_subscription_event_type_t event_type,
                                         uint32_t idx,
                                         void *userdata)
{
    PulseContext *self = (PulseContext *)(userdata);

    KLOG_DEBUG("Receive subscribe event. facility: %s, type: %s, idx: %d.",
               event2facility(event_type).c_str(),
               event2type(event_type).c_str(),
               idx);

    auto type = (event_type & PA_SUBSCRIPTION_EVENT_TYPE_MASK);

    switch (event_type & PA_SUBSCRIPTION_EVENT_FACILITY_MASK)
    {
    case PA_SUBSCRIPTION_EVENT_SERVER:
        self->load_server_info();
        break;
    case PA_SUBSCRIPTION_EVENT_CARD:
        if (type == PA_SUBSCRIPTION_EVENT_REMOVE)
        {
            self->card_info_removed_.emit(idx);
        }
        else
        {
            self->load_card_info(idx);
        }
        break;
    case PA_SUBSCRIPTION_EVENT_SINK:
        if (type == PA_SUBSCRIPTION_EVENT_REMOVE)
        {
            self->sink_info_removed_.emit(idx);
        }
        else
        {
            self->load_sink_info(idx);
        }
        break;
    case PA_SUBSCRIPTION_EVENT_SINK_INPUT:
        if (type == PA_SUBSCRIPTION_EVENT_REMOVE)
        {
            self->sink_input_info_removed_.emit(idx);
        }
        else
        {
            self->load_sink_input_info(idx);
        }
        break;
    case PA_SUBSCRIPTION_EVENT_SOURCE:
        if (type == PA_SUBSCRIPTION_EVENT_REMOVE)
        {
            self->source_info_removed_.emit(idx);
        }
        else
        {
            self->load_source_info(idx);
        }
        break;
    case PA_SUBSCRIPTION_EVENT_SOURCE_OUTPUT:
        if (type == PA_SUBSCRIPTION_EVENT_REMOVE)
        {
            self->source_output_info_removed_.emit(idx);
        }
        else
        {
            self->load_source_output_info(idx);
        }
        break;
    default:
        break;
    }
}

void PulseContext::on_pulse_server_info_cb(pa_context *context, const pa_server_info *server_info, void *userdata)
{
    PulseContext *self = (PulseContext *)(userdata);
    RETURN_IF_FALSE(self != NULL && self->context_ == context);

    self->server_info_changed_.emit(server_info);
    // 服务器信息的回调函数是最后一个回调函数
    if (self->connection_state_ == PulseConnectionState::PULSE_CONNECTION_LOADING)
    {
        self->set_connection_state(PulseConnectionState::PULSE_CONNECTION_CONNECTED);
    }
}

// 只有在加载状态时才需要调用load_list_finished，因为服务器信息(server info)需要等待sink/source等请求完毕后才进行加载
#define ON_PULSE_INFO_CB(info)                                                                                   \
    void PulseContext::on_pulse_##info##_cb(pa_context *context, const pa_##info *info, int eol, void *userdata) \
    {                                                                                                            \
        KLOG_PROFILE("eol: %d.", eol);                                                                           \
        PulseContext *self = (PulseContext *)(userdata);                                                         \
        RETURN_IF_FALSE(self != NULL && self->context_ == context);                                              \
                                                                                                                 \
        if (eol)                                                                                                 \
        {                                                                                                        \
            if (self->connection_state_ == PulseConnectionState::PULSE_CONNECTION_LOADING)                       \
            {                                                                                                    \
                self->load_list_finished();                                                                      \
            }                                                                                                    \
            return;                                                                                              \
        }                                                                                                        \
        self->info##_changed_.emit(info);                                                                        \
    }

ON_PULSE_INFO_CB(card_info)
ON_PULSE_INFO_CB(sink_info)
ON_PULSE_INFO_CB(sink_input_info)
ON_PULSE_INFO_CB(source_info)
ON_PULSE_INFO_CB(source_output_info)

std::string PulseContext::get_default_app_name()
{
    char name_buf[256] = {0};

    auto app_name = Glib::get_application_name();
    RETURN_VAL_IF_TRUE(!app_name.empty(), app_name);

    if (pa_get_binary_name(name_buf, sizeof(name_buf)) != NULL)
    {
        return std::string(name_buf);
    }

    return PROJECT_NAME;
}
}  // namespace Kiran