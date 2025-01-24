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

#include <QDBusContext>
#include <QMap>
#include <QSharedPointer>
#include "pulse/pulse-backend.h"

class AudioAdaptor;

namespace Kiran
{
class AudioDevice;
class AudioStream;
class PulseBackend;
class PulseSink;
class PulseSource;
class PulseSinkInput;
class PulseSourceOutput;

class AudioManager : public QObject,
                     protected QDBusContext
{
    Q_OBJECT
    Q_PROPERTY(uint state READ getState)

public:
    AudioManager(PulseBackend *backend);
    virtual ~AudioManager();

    static AudioManager *getInstance() { return m_instance; };

    static void globalInit(PulseBackend *backend);

    static void globalDeinit() { delete m_instance; };

    QSharedPointer<AudioDevice> getSink(uint32_t index) { return m_sinks.value(index); };
    QSharedPointer<AudioDevice> getSource(uint32_t index) { return m_sources.value(index); };

    QSharedPointer<AudioStream> getSinkInput(uint32_t index) { return m_sinkInputs.value(index); };
    QSharedPointer<AudioStream> getSourceOutput(uint32_t index) { return m_sourceOutputs.value(index); };

public:
    uint getState() const;

public Q_SLOTS:
    // 获取所有声卡设备
    QString GetCards();
    // 获取默认sink(扬声器)
    QString GetDefaultSink();
    // 获取默认source(话筒)
    QString GetDefaultSource();
    // 根据index获取sink
    QString GetSink(uint index);
    // 根据index获取sink input，sink input一般由应用程序创建，sink input会连接一个sink，通过sink input可以控制应用程序的输出音量
    QString GetSinkInput(uint index);
    // 获取所有sink input
    QStringList GetSinkInputs();
    // 获取所有sink
    QStringList GetSinks();
    // 根据index获取source
    QString GetSource(uint index);
    // 根据index获取source output，souce output一般由应用程序创建，source output会连接一个source，通过source output可以控制应用程序的输入音量
    QString GetSourceOutput(uint index);
    // 获取程序控制的source
    QStringList GetSourceOutputs();
    // 获取所有source
    QStringList GetSources();
    // 设置默认sink(扬声器)
    void SetDefaultSink(uint sinkIndex);
    // 设置默认source(话筒)
    void SetDefaultSource(uint sourceIndex);
Q_SIGNALS:  // SIGNALS
    void DefaultSinkChange(uint index);
    void DefaultSourceChange(uint index);
    void SinkAdded(uint index);
    void SinkDelete(uint index);
    void SinkInputAdded(uint index);
    void SinkInputDelete(uint index);
    void SourceAdded(uint index);
    void SourceDelete(uint index);
    void SourceOutputAdded(uint index);
    void SourceOutputDelete(uint index);

private:
    void init();

    // 加载sink/source/sink input/source output等
    void addComponents();
    QSharedPointer<AudioDevice> addSink(QSharedPointer<PulseSink> pulseSink);
    QSharedPointer<AudioDevice> addSource(QSharedPointer<PulseSource> pulseSource);
    QSharedPointer<AudioStream> addSinkInput(QSharedPointer<PulseSinkInput> pulseSinkInput);
    QSharedPointer<AudioStream> addSourceOutput(QSharedPointer<PulseSourceOutput> pulseSourceOutput);
    //
    void delComponents();

    void processStateChanged(AudioState state);
    void processDefaultSinkChanged(QSharedPointer<PulseSink> pulse_sink);
    void processDefaultSourceChanged(QSharedPointer<PulseSource> pulse_source);
    void processSinkEvent(PulseSinkEvent event, QSharedPointer<PulseSink> pulse_sink);
    void processSinkInputEvent(PulseSinkInputEvent event, QSharedPointer<PulseSinkInput> pulse_sink_input);
    void processSourceEvent(PulseSourceEvent event, QSharedPointer<PulseSource> pulse_source);
    void processSourceOutputEvent(PulseSourceOutputEvent event, QSharedPointer<PulseSourceOutput> pulse_source_output);

private:
    static AudioManager *m_instance;
    AudioAdaptor *m_adaptor;
    PulseBackend *m_backend;

    QMap<uint32_t, QSharedPointer<AudioDevice>> m_sinks;
    QMap<uint32_t, QSharedPointer<AudioDevice>> m_sources;
    QMap<uint32_t, QSharedPointer<AudioStream>> m_sinkInputs;
    QMap<uint32_t, QSharedPointer<AudioStream>> m_sourceOutputs;
};
}  // namespace Kiran