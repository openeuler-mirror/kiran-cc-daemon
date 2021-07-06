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

#include <audio_dbus_stub.h>

#include "plugins/audio/pulse/pulse-backend.h"

namespace Kiran
{
class AudioDevice;
class AudioStream;

class AudioManager : public SessionDaemon::AudioStub
{
public:
    AudioManager(PulseBackend *backend);
    virtual ~AudioManager();

    static AudioManager *get_instance() { return instance_; };

    static void global_init(PulseBackend *backend);

    static void global_deinit() { delete instance_; };

    std::shared_ptr<AudioDevice> get_sink(uint32_t index) { return MapHelper::get_value(this->sinks_, index); };
    std::shared_ptr<AudioDevice> get_source(uint32_t index) { return MapHelper::get_value(this->sources_, index); };

    std::shared_ptr<AudioStream> get_sink_input(uint32_t index) { return MapHelper::get_value(this->sink_inputs_, index); };
    std::shared_ptr<AudioStream> get_source_output(uint32_t index) { return MapHelper::get_value(this->source_outputs_, index); };

protected:
    // 获取默认sink(扬声器)
    virtual void GetDefaultSink(MethodInvocation &invocation);
    // 设置默认sink(扬声器)
    virtual void SetDefaultSink(guint32 sink_index, MethodInvocation &invocation);
    // 获取所有sink
    virtual void GetSinks(MethodInvocation &invocation);
    // 根据index获取sink
    virtual void GetSink(guint32 index, MethodInvocation &invocation);
    // 获取默认source(话筒)
    virtual void GetDefaultSource(MethodInvocation &invocation);
    // 设置默认source(话筒)
    virtual void SetDefaultSource(guint32 source_index, MethodInvocation &invocation);
    // 获取所有source
    virtual void GetSources(MethodInvocation &invocation);
    // 根据index获取source
    virtual void GetSource(guint32 index, MethodInvocation &invocation);
    // 获取所有sink input
    virtual void GetSinkInputs(MethodInvocation &invocation);
    // 根据index获取sink input，sink input一般由应用程序创建，sink input会连接一个sink，通过sink input可以控制应用程序的输出音量
    virtual void GetSinkInput(guint32 index, MethodInvocation &invocation);
    // 获取程序控制的source
    virtual void GetSourceOutputs(MethodInvocation &invocation);
    // 根据index获取source output，souce output一般由应用程序创建，source output会连接一个source，通过source output可以控制应用程序的输入音量
    virtual void GetSourceOutput(guint32 index, MethodInvocation &invocation);

    virtual bool state_setHandler(guint32 value) { return true; };
    virtual guint32 state_get();

private:
    void init();

    // 加载sink/source/sink input/source output等
    void add_components();
    std::shared_ptr<AudioDevice> add_sink(std::shared_ptr<PulseSink> pulse_sink);
    std::shared_ptr<AudioDevice> add_source(std::shared_ptr<PulseSource> pulse_source);
    std::shared_ptr<AudioStream> add_sink_input(std::shared_ptr<PulseSinkInput> pulse_sink_input);
    std::shared_ptr<AudioStream> add_source_output(std::shared_ptr<PulseSourceOutput> pulse_source_output);
    //
    void del_components();

    void on_state_changed_cb(AudioState state);
    void on_default_sink_changed_cb(std::shared_ptr<PulseSink> pulse_sink);
    void on_default_source_changed_cb(std::shared_ptr<PulseSource> pulse_source);
    void on_sink_event_cb(PulseSinkEvent event, std::shared_ptr<PulseSink> pulse_sink);
    void on_sink_input_event_cb(PulseSinkInputEvent event, std::shared_ptr<PulseSinkInput> pulse_sink_input);
    void on_source_event_cb(PulseSourceEvent event, std::shared_ptr<PulseSource> pulse_source);
    void on_source_output_event_cb(PulseSourceOutputEvent event, std::shared_ptr<PulseSourceOutput> pulse_source_output);

    void on_bus_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name);
    void on_name_acquired(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name);
    void on_name_lost(const Glib::RefPtr<Gio::DBus::Connection> &connect, Glib::ustring name);

private:
    static AudioManager *instance_;
    PulseBackend *backend_;

    std::map<uint32_t, std::shared_ptr<AudioDevice>> sinks_;
    std::map<uint32_t, std::shared_ptr<AudioDevice>> sources_;

    std::map<uint32_t, std::shared_ptr<AudioStream>> sink_inputs_;
    std::map<uint32_t, std::shared_ptr<AudioStream>> source_outputs_;

    uint32_t dbus_connect_id_;
    uint32_t object_register_id_;
};
}  // namespace Kiran