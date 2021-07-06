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


#include "plugins/audio/pulse/pulse-backend.h"

namespace Kiran
{
PulseBackend::PulseBackend() : state_(AudioState::AUDIO_STATE_IDLE),
                               connected_once_(false),
                               reconnection_handle_(0)
{
    this->context_ = std::make_shared<PulseContext>();
}

PulseBackend::~PulseBackend()
{
}

PulseBackend *PulseBackend::instance_ = nullptr;
void PulseBackend::global_init()
{
    instance_ = new PulseBackend();
    instance_->init();
}

std::shared_ptr<PulseSink> PulseBackend::get_sink_by_name(const std::string &name)
{
    for (auto &iter : this->sinks_)
    {
        if (iter.second->get_name() == name)
        {
            return iter.second;
        }
    }
    return nullptr;
}

std::shared_ptr<PulseSource> PulseBackend::get_source_by_name(const std::string &name)
{
    for (auto &iter : this->sources_)
    {
        if (iter.second->get_name() == name)
        {
            return iter.second;
        }
    }
    return nullptr;
}

bool PulseBackend::set_default_sink(std::shared_ptr<PulseSink> sink)
{
    RETURN_VAL_IF_FALSE(sink, false);

    RETURN_VAL_IF_FALSE(this->context_->set_default_sink(sink->get_name()), false);

    // PULSE_SET_PENDING_SINK_NULL(pulse);
    // PULSE_SET_DEFAULT_SINK(pulse, stream);

    return true;
}

bool PulseBackend::set_default_source(std::shared_ptr<PulseSource> source)
{
    RETURN_VAL_IF_FALSE(source, false);

    RETURN_VAL_IF_FALSE(this->context_->set_default_source(source->get_name()), false);

    // PULSE_SET_PENDING_SINK_NULL(pulse);
    // PULSE_SET_DEFAULT_SINK(pulse, stream);

    return true;
}

bool PulseBackend::init()
{
    this->context_->signal_connection_state_changed().connect(sigc::mem_fun(this, &PulseBackend::on_connection_state_changed_cb));
    this->context_->signal_server_info_changed().connect(sigc::mem_fun(this, &PulseBackend::on_server_info_changed_cb));
    this->context_->signal_card_info_changed().connect(sigc::mem_fun(this, &PulseBackend::on_card_info_changed_cb));
    this->context_->signal_card_info_removed().connect(sigc::mem_fun(this, &PulseBackend::on_card_info_removed_cb));
    this->context_->signal_sink_info_changed().connect(sigc::mem_fun(this, &PulseBackend::on_sink_info_changed_cb));
    this->context_->signal_sink_info_removed().connect(sigc::mem_fun(this, &PulseBackend::on_sink_info_removed_cb));
    this->context_->signal_sink_input_info_changed().connect(sigc::mem_fun(this, &PulseBackend::on_sink_input_info_changed_cb));
    this->context_->signal_sink_input_info_removed().connect(sigc::mem_fun(this, &PulseBackend::on_sink_input_info_removed_cb));
    this->context_->signal_source_info_changed().connect(sigc::mem_fun(this, &PulseBackend::on_source_info_changed_cb));
    this->context_->signal_source_info_removed().connect(sigc::mem_fun(this, &PulseBackend::on_source_info_removed_cb));
    this->context_->signal_source_output_info_changed().connect(sigc::mem_fun(this, &PulseBackend::on_source_output_info_changed_cb));
    this->context_->signal_source_output_info_removed().connect(sigc::mem_fun(this, &PulseBackend::on_source_output_info_removed_cb));

    this->set_state(AudioState::AUDIO_STATE_CONNECTING);

    if (!this->context_->connect(false))
    {
        this->set_state(AudioState::AUDIO_STATE_FAILED);
        return false;
    }

    return true;
}

void PulseBackend::set_state(AudioState state)
{
    KLOG_PROFILE("state: %d.", state);

    if (this->state_ != state)
    {
        this->state_ = state;
        this->state_changed_.emit(this->state_);
    }
}

bool PulseBackend::try_reconnection()
{
    KLOG_PROFILE("");

    if (this->context_->connect(true))
    {
        this->reconnection_handle_ = 0;
        return G_SOURCE_REMOVE;
    }
    return G_SOURCE_CONTINUE;
}

void PulseBackend::reset_data()
{
    this->server_info_ = PulseServerInfo();

    for (auto iter : this->cards_)
    {
        this->card_event_.emit(PulseCardEvent::PULSE_CARD_EVENT_DELETED, iter.second);
    }
    this->cards_.clear();

    for (auto iter : this->sinks_)
    {
        this->sink_event_.emit(PulseSinkEvent::PULSE_SINK_EVENT_DELETED, iter.second);
    }
    this->sinks_.clear();

    for (auto iter : this->sink_inputs_)
    {
        this->sink_input_event_.emit(PulseSinkInputEvent::PULSE_SINK_INPUT_EVENT_DELETED, iter.second);
    }
    this->sinks_.clear();

    for (auto iter : this->sources_)
    {
        this->source_event_.emit(PulseSourceEvent::PULSE_SOURCE_EVENT_DELETED, iter.second);
    }
    this->sources_.clear();

    for (auto iter : this->source_outputs_)
    {
        this->source_output_event_.emit(PulseSourceOutputEvent::PULSE_SOURCE_OUTPUT_EVENT_DELETED, iter.second);
    }
    this->source_outputs_.clear();
}

void PulseBackend::on_connection_state_changed_cb(PulseConnectionState connection_state)
{
    KLOG_PROFILE("connection state: %d.", connection_state);

    switch (connection_state)
    {
    case PulseConnectionState::PULSE_CONNECTION_DISCONNECTED:
    {
        // 如果之前已经成功连接过一次，此时突然断开了连接，则重新进行连接
        // 重新连接之前需要清理掉之前的数据，需要测试一下重启pulseaudio服务程序会不会出问题
        this->reset_data();

        if (this->connected_once_)
        {
            this->set_state(AudioState::AUDIO_STATE_CONNECTING);

            if (this->reconnection_handle_)
            {
                KLOG_DEBUG("The reconnection handle is already exist. handle: %d.", this->reconnection_handle_);
                break;
            }

            if (!this->context_->connect(true))
            {
                auto timeout_source = Glib::TimeoutSource::create(200);
                timeout_source->connect(sigc::mem_fun(this, &PulseBackend::try_reconnection));
                auto glib_context = Glib::wrap(g_main_context_get_thread_default());
                this->reconnection_handle_ = timeout_source->attach(glib_context);
            }
        }
        else
        {
            this->set_state(AudioState::AUDIO_STATE_FAILED);
        }
        break;
    }
    case PulseConnectionState::PULSE_CONNECTION_CONNECTING:
    case PulseConnectionState::PULSE_CONNECTION_AUTHORIZING:
    case PulseConnectionState::PULSE_CONNECTION_LOADING:
        this->set_state(AudioState::AUDIO_STATE_CONNECTING);
        break;
    case PulseConnectionState::PULSE_CONNECTION_CONNECTED:
    {
        this->connected_once_ = true;
        this->set_state(AudioState::AUDIO_STATE_READY);
        break;
    }
    default:
        break;
    }
}

void PulseBackend::on_server_info_changed_cb(const pa_server_info *server_info)
{
    RETURN_IF_FALSE(server_info != NULL);

    auto old_server_info = this->server_info_;
    this->server_info_ = PulseServerInfo{.user_name = POINTER_TO_STRING(server_info->user_name),
                                         .host_name = POINTER_TO_STRING(server_info->host_name),
                                         .server_version = POINTER_TO_STRING(server_info->server_version),
                                         .server_name = POINTER_TO_STRING(server_info->server_name),
                                         .sample_spec = server_info->sample_spec,
                                         .default_sink_name = POINTER_TO_STRING(server_info->default_sink_name),
                                         .default_source_name = POINTER_TO_STRING(server_info->default_source_name),
                                         .cookie = server_info->cookie};

    KLOG_DEBUG("Server info: username: %s, hostname: %s, server version: %s, "
               "server name: %s, default sink name: %s, default source name: %s, cookie: %d.",
               this->server_info_.user_name.c_str(),
               this->server_info_.host_name.c_str(),
               this->server_info_.server_version.c_str(),
               this->server_info_.server_name.c_str(),
               this->server_info_.default_sink_name.c_str(),
               this->server_info_.default_source_name.c_str(),
               this->server_info_.cookie);

    // 检测默认的sink是否发生变化
    if (old_server_info.default_sink_name != this->server_info_.default_sink_name)
    {
        if (this->server_info_.default_sink_name.empty())
        {
            this->default_sink_ = nullptr;
            this->default_sink_changed_.emit(this->default_sink_);
        }
        else
        {
            auto sink = this->get_sink_by_name(this->server_info_.default_sink_name);
            /* 当card profile发生变化时，on_server_info_changed_cb可能优先on_sink_info_changed_cb函数被调用，
               此时default sink可能还找不到（为空），这种情况则将信号延迟到on_sink_info_changed_cb时再进行处理。*/
            if (sink)
            {
                this->default_sink_ = sink;
                this->default_sink_changed_.emit(this->default_sink_);
            }
            else
            {
                this->context_->load_sink_info_by_name(this->server_info_.default_sink_name);
            }
        }
    }

    // 检测默认的source是否发生变化
    if (old_server_info.default_source_name != this->server_info_.default_source_name)
    {
        if (this->server_info_.default_source_name.empty())
        {
            this->default_source_ = nullptr;
            this->default_source_changed_.emit(this->default_source_);
        }
        else
        {
            auto source = this->get_source_by_name(this->server_info_.default_source_name);
            if (source)
            {
                this->default_source_ = source;
                this->default_source_changed_.emit(this->default_source_);
            }
            else
            {
                this->context_->load_source_info_by_name(this->server_info_.default_source_name);
            }
        }
    }
}

void PulseBackend::on_card_info_changed_cb(const pa_card_info *card_info)
{
    RETURN_IF_FALSE(card_info != NULL);

    KLOG_DEBUG("card changed, index: %d, name: %s.", card_info->index, card_info->name ? card_info->name : "NULL");

    auto card = this->get_card(card_info->index);

    if (card)
    {
        card->update(card_info);
        this->card_event_.emit(PulseCardEvent::PULSE_CARD_EVENT_CHANGED, card);
    }
    else
    {
        card = std::make_shared<PulseCard>(card_info);
        this->cards_.emplace(card_info->index, card);
        this->card_event_.emit(PulseCardEvent::PULSE_CARD_EVENT_ADDED, card);
    }
}

void PulseBackend::on_card_info_removed_cb(uint32_t index)
{
    KLOG_DEBUG("card removed, index: %d.", index);

    auto card = this->get_card(index);

    if (card)
    {
        this->card_event_.emit(PulseCardEvent::PULSE_CARD_EVENT_DELETED, card);
        this->cards_.erase(index);
    }
    else
    {
        KLOG_WARNING("The card index %d is not found.", index);
    }
}

void PulseBackend::on_sink_info_changed_cb(const pa_sink_info *sink_info)
{
    RETURN_IF_FALSE(sink_info != NULL);

    KLOG_DEBUG("sink changed, index: %d, name: %s.", sink_info->index, sink_info->name ? sink_info->name : "NULL");

    auto sink = this->get_sink(sink_info->index);

    if (sink)
    {
        sink->update(sink_info);
        this->sink_event_.emit(PulseSinkEvent::PULSE_SINK_EVENT_CHANGED, sink);
    }
    else
    {
        sink = std::make_shared<PulseSink>(this->context_, sink_info);
        this->sinks_.emplace(sink_info->index, sink);
        this->sink_event_.emit(PulseSinkEvent::PULSE_SINK_EVENT_ADDED, sink);
        /* sink一般情况下都会绑定一个card，部分的后端Device Driver（例如OSS）不存在card的概念，
           因此不确定sink是否在任何情况下都会绑定card，而且card和sink的初始化都是异步回调函数（不确定card是否先于sink回调？)，
           因此保险起见在这里加一个判空条件。*/
        // auto card = this->get_card(sink_info->card);
        // if (card)
        // {
        //     card->add_stream(sink);
        // }

        // 如果新增的是默认sink，则发送信号（延迟到此时进行处理）
        if (sink->get_name() == this->server_info_.default_sink_name)
        {
            this->default_sink_ = sink;
            this->default_sink_changed_.emit(this->default_sink_);
        }
    }
}

void PulseBackend::on_sink_info_removed_cb(uint32_t index)
{
    KLOG_DEBUG("sink removed, index: %d.", index);

    auto sink = this->get_sink(index);

    if (!sink)
    {
        KLOG_WARNING("The sink index %d is not found.", index);
        return;
    }

    // auto card = sink->get_card();
    this->sink_event_.emit(PulseSinkEvent::PULSE_SINK_EVENT_DELETED, sink);
    this->sinks_.erase(index);

    // if (card)
    // {
    //     card->remove_stream(sink);
    // }

    /*当card profile发生变化时，default sink可能会发生变化，因此这里先进行清理*/
    if (sink->get_name() == this->server_info_.default_sink_name)
    {
        this->default_sink_ = nullptr;
        this->default_sink_changed_.emit(this->default_sink_);
        this->context_->load_server_info();
    }
}

void PulseBackend::on_sink_input_info_changed_cb(const pa_sink_input_info *sink_input_info)
{
    RETURN_IF_FALSE(sink_input_info != NULL);

    KLOG_DEBUG("sink input changed, index: %d, name: %s.", sink_input_info->index, sink_input_info->name ? sink_input_info->name : "NULL");

    auto sink_input = this->get_sink_input(sink_input_info->index);

    if (sink_input)
    {
        sink_input->update(sink_input_info);
        this->sink_input_event_.emit(PulseSinkInputEvent::PULSE_SINK_INPUT_EVENT_CHANGED, sink_input);
    }
    else
    {
        sink_input = std::make_shared<PulseSinkInput>(this->context_, sink_input_info);
        this->sink_inputs_.emplace(sink_input_info->index, sink_input);
        this->sink_input_event_.emit(PulseSinkInputEvent::PULSE_SINK_INPUT_EVENT_ADDED, sink_input);
    }
}

void PulseBackend::on_sink_input_info_removed_cb(uint32_t index)
{
    KLOG_DEBUG("sink input removed, index: %d.", index);

    auto sink_input = this->get_sink_input(index);

    if (!sink_input)
    {
        KLOG_WARNING("The sink input index %d is not found.", index);
        return;
    }

    this->sink_input_event_.emit(PulseSinkInputEvent::PULSE_SINK_INPUT_EVENT_DELETED, sink_input);
    this->sink_inputs_.erase(index);
}

void PulseBackend::on_source_info_changed_cb(const pa_source_info *source_info)
{
    RETURN_IF_FALSE(source_info != NULL);

    KLOG_DEBUG("source changed, index: %d, name: %s.", source_info->index, source_info->name ? source_info->name : "NULL");

    auto source = this->get_source(source_info->index);

    if (source)
    {
        source->update(source_info);
        this->source_event_.emit(PulseSourceEvent::PULSE_SOURCE_EVENT_CHANGED, source);
    }
    else
    {
        source = std::make_shared<PulseSource>(this->context_, source_info);
        this->sources_.emplace(source_info->index, source);
        this->source_event_.emit(PulseSourceEvent::PULSE_SOURCE_EVENT_ADDED, source);

        // auto card = this->get_card(source_info->card);
        // if (card)
        // {
        //     card->add_stream(source);
        // }

        // 如果新增的是默认source，则发送信号（延迟到此时进行处理）
        if (source->get_name() == this->server_info_.default_source_name)
        {
            this->default_source_ = source;
            this->default_source_changed_.emit(this->default_source_);
        }
    }
}

void PulseBackend::on_source_info_removed_cb(uint32_t index)
{
    KLOG_DEBUG("source removed, index: %d.", index);

    auto source = this->get_source(index);

    if (!source)
    {
        KLOG_WARNING("The source index %d is not found.", index);
        return;
    }

    // auto card = source->get_card();
    this->source_event_.emit(PulseSourceEvent::PULSE_SOURCE_EVENT_DELETED, source);
    this->sources_.erase(index);

    // if (card)
    // {
    //     card->remove_stream(source);
    // }

    /*当card profile发生变化时，default source可能会发生变化，因此这里先进行清理*/
    if (source->get_name() == this->server_info_.default_source_name)
    {
        this->default_source_ = nullptr;
        this->default_source_changed_.emit(this->default_source_);
        this->context_->load_server_info();
    }
}

void PulseBackend::on_source_output_info_changed_cb(const pa_source_output_info *source_output_info)
{
    RETURN_IF_FALSE(source_output_info != NULL);

    KLOG_DEBUG("source output changed, index: %d, name: %s.", source_output_info->index, source_output_info->name ? source_output_info->name : "NULL");

    auto source_output = this->get_source_output(source_output_info->index);

    if (source_output)
    {
        source_output->update(source_output_info);
        this->source_output_event_.emit(PulseSourceOutputEvent::PULSE_SOURCE_OUTPUT_EVENT_CHANGED, source_output);
    }
    else
    {
        source_output = std::make_shared<PulseSourceOutput>(this->context_, source_output_info);
        this->source_outputs_.emplace(source_output_info->index, source_output);
        this->source_output_event_.emit(PulseSourceOutputEvent::PULSE_SOURCE_OUTPUT_EVENT_ADDED, source_output);
    }
}

void PulseBackend::on_source_output_info_removed_cb(uint32_t index)
{
    KLOG_DEBUG("source output removed, index: %d.", index);

    auto source_output = this->get_source_output(index);

    if (!source_output)
    {
        KLOG_WARNING("The source output index %d is not found.", index);
        return;
    }

    this->source_output_event_.emit(PulseSourceOutputEvent::PULSE_SOURCE_OUTPUT_EVENT_DELETED, source_output);
    this->source_outputs_.erase(index);
}
}  // namespace Kiran