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

#include "plugins/audio/audio-manager.h"
#include "audio-i.h"
#include "plugins/audio/audio-device.h"
#include "plugins/audio/audio-stream.h"

namespace Kiran
{
AudioManager::AudioManager(PulseBackend *backend) : backend_(backend),
                                                    dbus_connect_id_(0),
                                                    object_register_id_(0)
{
}

AudioManager::~AudioManager()
{
    if (this->dbus_connect_id_)
    {
        Gio::DBus::unown_name(this->dbus_connect_id_);
    }
}

AudioManager *AudioManager::instance_ = nullptr;
void AudioManager::global_init(PulseBackend *backend)
{
    instance_ = new AudioManager(backend);
    instance_->init();
}

void AudioManager::init()
{
    this->dbus_connect_id_ = Gio::DBus::own_name(Gio::DBus::BUS_TYPE_SESSION,
                                                 AUDIO_DBUS_NAME,
                                                 sigc::mem_fun(this, &AudioManager::on_bus_acquired),
                                                 sigc::mem_fun(this, &AudioManager::on_name_acquired),
                                                 sigc::mem_fun(this, &AudioManager::on_name_lost));

    this->backend_->signal_state_changed().connect(sigc::mem_fun(this, &AudioManager::on_state_changed_cb));
    this->backend_->signal_default_sink_changed().connect(sigc::mem_fun(this, &AudioManager::on_default_sink_changed_cb));
    this->backend_->signal_default_source_changed().connect(sigc::mem_fun(this, &AudioManager::on_default_source_changed_cb));

    this->backend_->signal_sink_event().connect(sigc::mem_fun(this, &AudioManager::on_sink_event_cb));
    this->backend_->signal_sink_input_event().connect(sigc::mem_fun(this, &AudioManager::on_sink_input_event_cb));
    this->backend_->signal_source_event().connect(sigc::mem_fun(this, &AudioManager::on_source_event_cb));
    this->backend_->signal_source_output_event().connect(sigc::mem_fun(this, &AudioManager::on_source_output_event_cb));
}

void AudioManager::GetDefaultSink(MethodInvocation &invocation)
{
    auto pulse_sink = this->backend_->get_default_sink();
    if (!pulse_sink)
    {
        KLOG_WARNING_AUDIO("The default sink is not set.");
        invocation.ret(Glib::ustring());
        return;
    }
    else
    {
        auto audio_sink = this->get_sink(pulse_sink->get_index());
        if (!audio_sink)
        {
            KLOG_WARNING_AUDIO("The audio sink isn't found, sink index: %d.", pulse_sink->get_index());
            invocation.ret(Glib::ustring());
        }
        else
        {
            invocation.ret(audio_sink->get_object_path());
        }
    }
}

void AudioManager::SetDefaultSink(guint32 sink_index, MethodInvocation &invocation)
{
    auto audio_sink = this->get_sink(sink_index);
    auto pulse_sink = this->backend_->get_sink(sink_index);
    if (!audio_sink || !pulse_sink)
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_AUDIO_DEFAULT_SINK_NOT_FOUND);
    }
    this->backend_->set_default_sink(pulse_sink);
    invocation.ret();
}

void AudioManager::GetSinks(MethodInvocation &invocation)
{
    std::vector<Glib::ustring> sinks;
    for (auto iter : this->sinks_)
    {
        sinks.push_back(iter.second->get_object_path());
    }
    invocation.ret(sinks);
}

void AudioManager::GetSink(guint32 index, MethodInvocation &invocation)
{
    auto sink = this->get_sink(index);
    if (!sink)
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_AUDIO_SINK_NOT_FOUND);
    }
    invocation.ret(sink->get_object_path());
}

void AudioManager::GetDefaultSource(MethodInvocation &invocation)
{
    auto pulse_source = this->backend_->get_default_source();
    if (!pulse_source)
    {
        KLOG_WARNING_AUDIO("The default source is not set.");
        invocation.ret(Glib::ustring());
        return;
    }
    else
    {
        auto audio_source = this->get_source(pulse_source->get_index());
        if (!audio_source)
        {
            KLOG_WARNING_AUDIO("The audio source isn't found, source index: %d.", pulse_source->get_index());
            invocation.ret(Glib::ustring());
        }
        else
        {
            invocation.ret(audio_source->get_object_path());
        }
    }
}

void AudioManager::SetDefaultSource(guint32 source_index, MethodInvocation &invocation)
{
    auto audio_source = this->get_source(source_index);
    auto pulse_source = this->backend_->get_source(source_index);
    if (!audio_source || !pulse_source)
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_AUDIO_DEFAULT_SOURCE_NOT_FOUND);
    }
    this->backend_->set_default_source(pulse_source);
    invocation.ret();
}

void AudioManager::GetSources(MethodInvocation &invocation)
{
    std::vector<Glib::ustring> sources;
    for (auto iter : this->sources_)
    {
        sources.push_back(iter.second->get_object_path());
    }
    invocation.ret(sources);
}

void AudioManager::GetSource(guint32 index, MethodInvocation &invocation)
{
    auto source = this->get_source(index);
    if (!source)
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_AUDIO_SOURCE_NOT_FOUND);
    }
    invocation.ret(source->get_object_path());
}

void AudioManager::GetSinkInputs(MethodInvocation &invocation)
{
    std::vector<Glib::ustring> sink_inputs;
    for (auto iter : this->sink_inputs_)
    {
        sink_inputs.push_back(iter.second->get_object_path());
    }
    invocation.ret(sink_inputs);
}

void AudioManager::GetSinkInput(guint32 index, MethodInvocation &invocation)
{
    auto sink_input = this->get_sink_input(index);
    if (!sink_input)
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_AUDIO_SINK_INPUT_NOT_FOUND);
    }
    invocation.ret(sink_input->get_object_path());
}

void AudioManager::GetSourceOutputs(MethodInvocation &invocation)
{
    std::vector<Glib::ustring> source_outputs;
    for (auto iter : this->source_outputs_)
    {
        source_outputs.push_back(iter.second->get_object_path());
    }
    invocation.ret(source_outputs);
}

void AudioManager::GetSourceOutput(guint32 index, MethodInvocation &invocation)
{
    auto source_output = this->get_source_output(index);
    if (!source_output)
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_AUDIO_SOURCE_OUTPUT_NOT_FOUND);
    }
    invocation.ret(source_output->get_object_path());
}

void AudioManager::GetCards(MethodInvocation &invocation)
{
    Json::Value values;
    Json::FastWriter writer;

    uint32_t i = 0;
    for (auto pulse_card : this->backend_->get_cards())
    {
        values[i]["index"] = pulse_card->get_index();
        values[i]["name"] = pulse_card->get_name();

        i++;
    }

    auto result = writer.write(values);

    invocation.ret(result);
}

guint32 AudioManager::state_get()
{
    return this->backend_->get_state();
}

void AudioManager::add_components()
{
    for (auto pulse_sink : this->backend_->get_sinks())
    {
        this->add_sink(pulse_sink);
    }

    for (auto pulse_source : this->backend_->get_sources())
    {
        this->add_source(pulse_source);
    }

    for (auto pulse_sink_input : this->backend_->get_sink_inputs())
    {
        this->add_sink_input(pulse_sink_input);
    }

    for (auto pulse_source_output : this->backend_->get_source_outputs())
    {
        this->add_source_output(pulse_source_output);
    }
}

std::shared_ptr<AudioDevice> AudioManager::add_sink(std::shared_ptr<PulseSink> pulse_sink)
{
    RETURN_VAL_IF_FALSE(pulse_sink, nullptr);

    auto audio_sink = std::make_shared<AudioDevice>(pulse_sink);
    if (audio_sink->init(AUDIO_SINK_OBJECT_PATH))
    {
        auto iter = this->sinks_.emplace(audio_sink->index_get(), audio_sink);
        if (!iter.second)
        {
            KLOG_WARNING_AUDIO("The audio sink is already exist, sink index: %d.", audio_sink->index_get());
            return nullptr;
        }
        this->SinkAdded_signal.emit(audio_sink->index_get());
        return audio_sink;
    }
    else
    {
        KLOG_WARNING_AUDIO("Init sink failed, sink index: %d.", pulse_sink->get_index());
        return nullptr;
    }
    return nullptr;
}

std::shared_ptr<AudioDevice> AudioManager::add_source(std::shared_ptr<PulseSource> pulse_source)
{
    RETURN_VAL_IF_FALSE(pulse_source, nullptr);

    auto audio_source = std::make_shared<AudioDevice>(pulse_source);
    if (audio_source->init(AUDIO_SOURCE_OBJECT_PATH))
    {
        auto iter = this->sources_.emplace(audio_source->index_get(), audio_source);
        if (!iter.second)
        {
            KLOG_WARNING_AUDIO("The audio source is already exist, source index: %d.", audio_source->index_get());
            return nullptr;
        }
        this->SourceAdded_signal.emit(audio_source->index_get());
        return audio_source;
    }
    else
    {
        KLOG_WARNING_AUDIO("Init source failed, source index: %d.", pulse_source->get_index());
        return nullptr;
    }
    return nullptr;
}

std::shared_ptr<AudioStream> AudioManager::add_sink_input(std::shared_ptr<PulseSinkInput> pulse_sink_input)
{
    RETURN_VAL_IF_FALSE(pulse_sink_input, nullptr);

    auto audio_sink_input = std::make_shared<AudioStream>(pulse_sink_input);
    if (audio_sink_input->init(AUDIO_SINK_INPUT_OBJECT_PATH))
    {
        auto iter = this->sink_inputs_.emplace(audio_sink_input->index_get(), audio_sink_input);
        if (!iter.second)
        {
            KLOG_WARNING_AUDIO("The audio sink input is already exist, sink input index: %d.", audio_sink_input->index_get());
            return nullptr;
        }
        this->SinkInputAdded_signal.emit(audio_sink_input->index_get());
        return audio_sink_input;
    }
    else
    {
        KLOG_WARNING_AUDIO("Init sink input failed, sink input index: %d.", pulse_sink_input->get_index());
        return nullptr;
    }
    return nullptr;
}

std::shared_ptr<AudioStream> AudioManager::add_source_output(std::shared_ptr<PulseSourceOutput> pulse_source_output)
{
    RETURN_VAL_IF_FALSE(pulse_source_output, nullptr);

    auto audio_source_output = std::make_shared<AudioStream>(pulse_source_output);
    if (audio_source_output->init(AUDIO_SOURCE_OUTPUT_OBJECT_PATH))
    {
        auto iter = this->source_outputs_.emplace(audio_source_output->index_get(), audio_source_output);
        if (!iter.second)
        {
            KLOG_WARNING_AUDIO("The audio source output is already exist, source output index: %d.", audio_source_output->index_get());
            return nullptr;
        }
        this->SourceOutputAdded_signal.emit(audio_source_output->index_get());
        return audio_source_output;
    }
    else
    {
        KLOG_WARNING_AUDIO("Init source output failed, source output index: %d.", pulse_source_output->get_index());
        return nullptr;
    }
    return nullptr;
}

void AudioManager::del_components()
{
    for (auto iter : this->sinks_)
    {
        this->SinkDelete_signal.emit(iter.second->index_get());
    }
    this->sinks_.clear();

    for (auto iter : this->sources_)
    {
        this->SourceDelete_signal.emit(iter.second->index_get());
    }
    this->sources_.clear();

    for (auto iter : this->sink_inputs_)
    {
        this->SinkInputDelete_signal.emit(iter.second->index_get());
    }
    this->sink_inputs_.clear();

    for (auto iter : this->source_outputs_)
    {
        this->SourceOutputDelete_signal.emit(iter.second->index_get());
    }
    this->source_outputs_.clear();
}

void AudioManager::on_state_changed_cb(AudioState state)
{
    switch (state)
    {
    case AudioState::AUDIO_STATE_READY:
        this->add_components();
        break;
    case AudioState::AUDIO_STATE_CONNECTING:
    case AudioState::AUDIO_STATE_FAILED:
        this->del_components();
        break;
    default:
        break;
    }
    this->state_set(state);
}

void AudioManager::on_default_sink_changed_cb(std::shared_ptr<PulseSink> pulse_sink)
{
    RETURN_IF_TRUE(this->backend_->get_state() != AudioState::AUDIO_STATE_READY);

    if (pulse_sink)
    {
        this->DefaultSinkChange_signal.emit(pulse_sink->get_index());
    }
    else
    {
        this->DefaultSinkChange_signal.emit(PA_INVALID_INDEX);
    }
}

void AudioManager::on_default_source_changed_cb(std::shared_ptr<PulseSource> pulse_source)
{
    RETURN_IF_TRUE(this->backend_->get_state() != AudioState::AUDIO_STATE_READY);

    if (pulse_source)
    {
        this->DefaultSourceChange_signal.emit(pulse_source->get_index());
    }
    else
    {
        this->DefaultSourceChange_signal.emit(PA_INVALID_INDEX);
    }
}

void AudioManager::on_sink_event_cb(PulseSinkEvent event, std::shared_ptr<PulseSink> pulse_sink)
{
    RETURN_IF_TRUE(this->backend_->get_state() != AudioState::AUDIO_STATE_READY);

    switch (event)
    {
    case PulseSinkEvent::PULSE_SINK_EVENT_DELETED:
    {
        RETURN_IF_FALSE(pulse_sink);
        if (!this->sinks_.erase(pulse_sink->get_index()))
        {
            KLOG_WARNING_AUDIO("Not found audio sink: %d.", pulse_sink->get_index());
        }
        else
        {
            this->SinkDelete_signal.emit(pulse_sink->get_index());
        }
        break;
    }
    case PulseSinkEvent::PULSE_SINK_EVENT_ADDED:
        this->add_sink(pulse_sink);
        break;
    default:
        break;
    }
}

void AudioManager::on_sink_input_event_cb(PulseSinkInputEvent event, std::shared_ptr<PulseSinkInput> pulse_sink_input)
{
    RETURN_IF_TRUE(this->backend_->get_state() != AudioState::AUDIO_STATE_READY);

    switch (event)
    {
    case PulseSinkInputEvent::PULSE_SINK_INPUT_EVENT_DELETED:
    {
        RETURN_IF_FALSE(pulse_sink_input);
        if (!this->sink_inputs_.erase(pulse_sink_input->get_index()))
        {
            KLOG_WARNING_AUDIO("Not found audio sink input: %d.", pulse_sink_input->get_index());
        }
        else
        {
            this->SinkInputDelete_signal.emit(pulse_sink_input->get_index());
        }
        break;
    }
    case PulseSinkInputEvent::PULSE_SINK_INPUT_EVENT_ADDED:
    {
        this->add_sink_input(pulse_sink_input);
        break;
    }
    default:
        break;
    }
}

void AudioManager::on_source_event_cb(PulseSourceEvent event, std::shared_ptr<PulseSource> pulse_source)
{
    RETURN_IF_TRUE(this->backend_->get_state() != AudioState::AUDIO_STATE_READY);

    switch (event)
    {
    case PulseSourceEvent::PULSE_SOURCE_EVENT_DELETED:
    {
        RETURN_IF_FALSE(pulse_source);
        if (!this->sources_.erase(pulse_source->get_index()))
        {
            KLOG_WARNING_AUDIO("Not found audio source: %d.", pulse_source->get_index());
        }
        else
        {
            this->SourceDelete_signal.emit(pulse_source->get_index());
        }
        break;
    }
    case PulseSourceEvent::PULSE_SOURCE_EVENT_ADDED:
        this->add_source(pulse_source);
        break;
    default:
        break;
    }
}

void AudioManager::on_source_output_event_cb(PulseSourceOutputEvent event, std::shared_ptr<PulseSourceOutput> pulse_source_output)
{
    RETURN_IF_TRUE(this->backend_->get_state() != AudioState::AUDIO_STATE_READY);

    switch (event)
    {
    case PulseSourceOutputEvent::PULSE_SOURCE_OUTPUT_EVENT_DELETED:
    {
        RETURN_IF_FALSE(pulse_source_output);
        if (!this->source_outputs_.erase(pulse_source_output->get_index()))
        {
            KLOG_WARNING_AUDIO("Not found audio source output: %d.", pulse_source_output->get_index());
        }
        else
        {
            this->SourceOutputDelete_signal.emit(pulse_source_output->get_index());
        }
        break;
    }
    case PulseSourceOutputEvent::PULSE_SOURCE_OUTPUT_EVENT_ADDED:
        this->add_source_output(pulse_source_output);
        break;
    default:
        break;
    }
}

void AudioManager::on_bus_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name)
{
    if (!connect)
    {
        KLOG_WARNING_AUDIO("Failed to connect dbus with %s", name.c_str());
        return;
    }
    try
    {
        this->object_register_id_ = this->register_object(connect, AUDIO_OBJECT_PATH);
    }
    catch (const Glib::Error &e)
    {
        KLOG_WARNING_AUDIO("Register object_path %s fail: %s.", AUDIO_OBJECT_PATH, e.what().c_str());
    }
}

void AudioManager::on_name_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name)
{
    KLOG_DEBUG_AUDIO("Success to register dbus name: %s", name.c_str());
}

void AudioManager::on_name_lost(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name)
{
    KLOG_WARNING_AUDIO("Failed to register dbus name: %s", name.c_str());
}

}  // namespace Kiran