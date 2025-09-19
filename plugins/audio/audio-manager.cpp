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

#include "audio-manager.h"
#include <QDBusConnection>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include "audio-device.h"
#include "audio-i.h"
#include "audio-stream.h"
#include "audioadaptor.h"
#include "lib/base/base.h"
#include "pulse/pulse-backend.h"
#include "pulse/pulse-card.h"
#include "pulse/pulse-sink-input.h"
#include "pulse/pulse-sink.h"
#include "pulse/pulse-source-output.h"
#include "pulse/pulse-source.h"

namespace Kiran
{
AudioManager::AudioManager(PulseBackend *backend) : m_backend(backend)
{
    m_adaptor = new AudioAdaptor(this);
}

AudioManager::~AudioManager()
{
}

AudioManager *AudioManager::m_instance = nullptr;
void AudioManager::globalInit(PulseBackend *backend)
{
    m_instance = new AudioManager(backend);
    m_instance->init();
}

void AudioManager::init()
{
    connect(m_backend, &PulseBackend::stateChanged, this, &AudioManager::processStateChanged);
    connect(m_backend, &PulseBackend::defaultSinkChanged, this, &AudioManager::processDefaultSinkChanged);
    connect(m_backend, &PulseBackend::defaultSourceChanged, this, &AudioManager::processDefaultSourceChanged);
    connect(m_backend, &PulseBackend::sinkEvent, this, &AudioManager::processSinkEvent);
    connect(m_backend, &PulseBackend::sinkInputEvent, this, &AudioManager::processSinkInputEvent);
    connect(m_backend, &PulseBackend::sourceEvent, this, &AudioManager::processSourceEvent);
    connect(m_backend, &PulseBackend::sourceOutputEvent, this, &AudioManager::processSourceOutputEvent);

    auto sessionConnection = QDBusConnection::sessionBus();
    if (!sessionConnection.registerService(AUDIO_DBUS_NAME))
    {
        KLOG_WARNING(audio) << "Failed to register dbus name:" << AUDIO_DBUS_NAME;
        return;
    }

    if (!sessionConnection.registerObject(AUDIO_OBJECT_PATH, AUDIO_DBUS_INTERFACE_NAME, this))
    {
        KLOG_ERROR(audio) << "Can't register object:" << sessionConnection.lastError();
        return;
    }
}

#define SEND_PROPERTY_NOTIFY(property, propertyHump)                          \
    QVariantMap changedProperties;                                            \
    changedProperties.insert(QStringLiteral(#property), get##propertyHump()); \
                                                                              \
    QDBusMessage signalMessage = QDBusMessage::createSignal(                  \
        AUDIO_OBJECT_PATH,                                                    \
        QStringLiteral("org.freedesktop.DBus.Properties"),                    \
        QStringLiteral("PropertiesChanged"));                                 \
                                                                              \
    signalMessage.setArguments({                                              \
        AUDIO_DBUS_INTERFACE_NAME,                                            \
        changedProperties,                                                    \
        QStringList(),                                                        \
    });                                                                       \
    QDBusConnection::sessionBus().send(signalMessage);

uint AudioManager::getState() const
{
    return m_backend->getState();
}

QString AudioManager::GetCards()
{
    QJsonArray jsonCards;

    for (auto card : m_backend->getCards())
    {
        QJsonObject jsonCard;
        jsonCard["index"] = int(card->getIndex());
        jsonCard["name"] = card->getName();
        jsonCards.append(jsonCard);
    }
    return QString(QJsonDocument(jsonCards).toJson(QJsonDocument::Compact));
}

QString AudioManager::GetDefaultSink()
{
    auto pulseSink = m_backend->getDefaultSink();
    if (!pulseSink)
    {
        KLOG_WARNING(audio) << "The default sink is not set.";
        return QString();
    }
    else
    {
        auto audioSink = getSink(pulseSink->getIndex());
        if (!audioSink)
        {
            KLOG_WARNING(audio) << "The audio sink isn't found, sink index" << pulseSink->getIndex();
            return QString();
        }
        else
        {
            return audioSink->getObjectPath();
        }
    }
}

QString AudioManager::GetDefaultSource()
{
    auto pulseSource = m_backend->getDefaultSource();
    if (!pulseSource)
    {
        KLOG_WARNING(audio) << "The default source is not set.";
        return QString();
    }
    else
    {
        auto audioSource = getSource(pulseSource->getIndex());
        if (!audioSource)
        {
            KLOG_WARNING(audio) << "The audio source isn't found, source index" << pulseSource->getIndex();
            return QString();
        }
        else
        {
            return audioSource->getObjectPath();
        }
    }
}

QString AudioManager::GetSink(uint index)
{
    auto sink = getSink(index);
    if (!sink)
    {
        DBUS_ERROR_REPLY_AND_RETVAL(QString(), CCErrorCode::ERROR_AUDIO_SINK_NOT_FOUND);
    }
    return sink->getObjectPath();
}

QString AudioManager::GetSinkInput(uint index)
{
    auto sinkInput = getSinkInput(index);
    if (!sinkInput)
    {
        DBUS_ERROR_REPLY_AND_RETVAL(QString(), CCErrorCode::ERROR_AUDIO_SINK_INPUT_NOT_FOUND);
    }
    return sinkInput->getObjectPath();
}

QStringList AudioManager::GetSinkInputs()
{
    QStringList sinkInputs;
    for (auto sinkInput : m_sinkInputs)
    {
        sinkInputs.push_back(sinkInput->getObjectPath());
    }
    return sinkInputs;
}

QStringList AudioManager::GetSinks()
{
    QStringList sinks;
    for (auto sink : m_sinks)
    {
        sinks.push_back(sink->getObjectPath());
    }
    return sinks;
}

QString AudioManager::GetSource(uint index)
{
    auto source = getSource(index);
    if (!source)
    {
        DBUS_ERROR_REPLY_AND_RETVAL(QString(), CCErrorCode::ERROR_AUDIO_SOURCE_NOT_FOUND);
    }
    return source->getObjectPath();
}

QString AudioManager::GetSourceOutput(uint index)
{
    auto sourceOutput = getSourceOutput(index);
    if (!sourceOutput)
    {
        DBUS_ERROR_REPLY_AND_RETVAL(QString(), CCErrorCode::ERROR_AUDIO_SOURCE_OUTPUT_NOT_FOUND);
    }
    return sourceOutput->getObjectPath();
}

QStringList AudioManager::GetSourceOutputs()
{
    QStringList sourceOutputs;
    for (auto sourceOutput : m_sourceOutputs)
    {
        sourceOutputs.push_back(sourceOutput->getObjectPath());
    }
    return sourceOutputs;
}

QStringList AudioManager::GetSources()
{
    QStringList sources;
    for (auto source : m_sources)
    {
        sources.push_back(source->getObjectPath());
    }
    return sources;
}

void AudioManager::SetDefaultSink(uint sinkIndex)
{
    auto audioSink = getSink(sinkIndex);
    auto pulseSink = m_backend->getSink(sinkIndex);
    if (!audioSink || !pulseSink)
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_AUDIO_DEFAULT_SINK_NOT_FOUND);
    }
    m_backend->setDefaultSink(pulseSink);
}

void AudioManager::SetDefaultSource(uint sourceIndex)
{
    auto audioSource = getSource(sourceIndex);
    auto pulseSource = m_backend->getSource(sourceIndex);
    if (!audioSource || !pulseSource)
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_AUDIO_DEFAULT_SOURCE_NOT_FOUND);
    }
    m_backend->setDefaultSource(pulseSource);
}

void AudioManager::addComponents()
{
    for (auto pulseSink : m_backend->getSinks())
    {
        addSink(pulseSink);
    }

    for (auto pulseSource : m_backend->getSources())
    {
        addSource(pulseSource);
    }

    for (auto pulseSinkInput : m_backend->getSinkInputs())
    {
        addSinkInput(pulseSinkInput);
    }

    for (auto pulseSourceOutput : m_backend->getSourceOutputs())
    {
        addSourceOutput(pulseSourceOutput);
    }
}

QSharedPointer<AudioDevice> AudioManager::addSink(QSharedPointer<PulseSink> pulseSink)
{
    RETURN_VAL_IF_FALSE(pulseSink, nullptr);

    auto audioSink = QSharedPointer<AudioDevice>::create(pulseSink);
    auto sinkIndex = audioSink->getIndex();
    if (audioSink->init(AUDIO_SINK_OBJECT_PATH))
    {
        if (m_sinks.contains(sinkIndex))
        {
            KLOG_WARNING(audio) << "The audio sink is already exist, sink index" << sinkIndex;
            return nullptr;
        }
        else
        {
            m_sinks.insert(sinkIndex, audioSink);
        }
        Q_EMIT SinkAdded(sinkIndex);
        return audioSink;
    }
    else
    {
        KLOG_WARNING(audio) << "Init sink failed, sink index" << sinkIndex;
        return nullptr;
    }
    return nullptr;
}

QSharedPointer<AudioDevice> AudioManager::addSource(QSharedPointer<PulseSource> pulseSource)
{
    RETURN_VAL_IF_FALSE(pulseSource, nullptr);

    auto audioSource = QSharedPointer<AudioDevice>::create(pulseSource);
    auto sourceIndex = audioSource->getIndex();

    if (audioSource->init(AUDIO_SOURCE_OBJECT_PATH))
    {
        if (m_sources.contains(sourceIndex))
        {
            KLOG_WARNING(audio) << "The audio source is already exist, source index" << sourceIndex;
            return nullptr;
        }
        else
        {
            m_sources.insert(sourceIndex, audioSource);
        }
        Q_EMIT SourceAdded(sourceIndex);
        return audioSource;
    }
    else
    {
        KLOG_WARNING(audio) << "Init source failed, source index" << sourceIndex;
        return nullptr;
    }
    return nullptr;
}

QSharedPointer<AudioStream> AudioManager::addSinkInput(QSharedPointer<PulseSinkInput> pulseSinkInput)
{
    RETURN_VAL_IF_FALSE(pulseSinkInput, nullptr);

    auto audioSinkInput = QSharedPointer<AudioStream>::create(pulseSinkInput);
    auto sinkInputIndex = audioSinkInput->getIndex();
    if (audioSinkInput->init(AUDIO_SINK_INPUT_OBJECT_PATH))
    {
        if (m_sinkInputs.contains(sinkInputIndex))
        {
            KLOG_WARNING(audio) << "The audio sink input is already exist, sink input index" << sinkInputIndex;
            return nullptr;
        }
        else
        {
            m_sinkInputs.insert(sinkInputIndex, audioSinkInput);
        }
        Q_EMIT SinkInputAdded(sinkInputIndex);
        return audioSinkInput;
    }
    else
    {
        KLOG_WARNING(audio) << "Init sink input failed, sink input index" << sinkInputIndex;
        return nullptr;
    }
    return nullptr;
}

QSharedPointer<AudioStream> AudioManager::addSourceOutput(QSharedPointer<PulseSourceOutput> pulseSourceOutput)
{
    RETURN_VAL_IF_FALSE(pulseSourceOutput, nullptr);

    auto audioSourceOutput = QSharedPointer<AudioStream>::create(pulseSourceOutput);
    auto sourceOutputIndex = audioSourceOutput->getIndex();
    if (audioSourceOutput->init(AUDIO_SOURCE_OUTPUT_OBJECT_PATH))
    {
        if (m_sourceOutputs.contains(sourceOutputIndex))
        {
            KLOG_WARNING(audio) << "The audio source output is already exist, source output index" << sourceOutputIndex;
            return nullptr;
        }
        else
        {
            m_sourceOutputs.insert(sourceOutputIndex, audioSourceOutput);
        }
        Q_EMIT SourceOutputAdded(sourceOutputIndex);
        return audioSourceOutput;
    }
    else
    {
        KLOG_WARNING(audio) << "Init source output failed, source output index" << sourceOutputIndex;
        return nullptr;
    }
    return nullptr;
}

void AudioManager::delComponents()
{
    for (auto sink : m_sinks)
    {
        Q_EMIT SinkDelete(sink->getIndex());
    }
    m_sinks.clear();

    for (auto source : m_sources)
    {
        Q_EMIT SourceDelete(source->getIndex());
    }
    m_sources.clear();

    for (auto sinkInput : m_sinkInputs)
    {
        Q_EMIT SinkInputDelete(sinkInput->getIndex());
    }
    m_sinkInputs.clear();

    for (auto sourceOutput : m_sourceOutputs)
    {
        Q_EMIT SourceOutputDelete(sourceOutput->getIndex());
    }
    m_sourceOutputs.clear();
}

void AudioManager::processStateChanged(AudioState state)
{
    switch (state)
    {
    case AudioState::AUDIO_STATE_READY:
        addComponents();
        break;
    case AudioState::AUDIO_STATE_CONNECTING:
    case AudioState::AUDIO_STATE_FAILED:
        delComponents();
        break;
    default:
        break;
    }

    SEND_PROPERTY_NOTIFY(state, State)
}

void AudioManager::processDefaultSinkChanged(QSharedPointer<PulseSink> pulseSink)
{
    RETURN_IF_TRUE(m_backend->getState() != AudioState::AUDIO_STATE_READY);

    if (pulseSink)
    {
        Q_EMIT DefaultSinkChange(pulseSink->getIndex());
    }
    else
    {
        Q_EMIT DefaultSinkChange(PA_INVALID_INDEX);
    }
}

void AudioManager::processDefaultSourceChanged(QSharedPointer<PulseSource> pulseSource)
{
    RETURN_IF_TRUE(m_backend->getState() != AudioState::AUDIO_STATE_READY);

    if (pulseSource)
    {
        Q_EMIT DefaultSourceChange(pulseSource->getIndex());
    }
    else
    {
        Q_EMIT DefaultSourceChange(PA_INVALID_INDEX);
    }
}

void AudioManager::processSinkEvent(PulseSinkEvent event, QSharedPointer<PulseSink> pulseSink)
{
    RETURN_IF_TRUE(m_backend->getState() != AudioState::AUDIO_STATE_READY);

    switch (event)
    {
    case PulseSinkEvent::PULSE_SINK_EVENT_DELETED:
    {
        RETURN_IF_FALSE(pulseSink);
        auto pulseSinkIndex = pulseSink->getIndex();
        if (m_sinks.remove(pulseSinkIndex) == 0)
        {
            KLOG_WARNING(audio) << "Not found audio sink" << pulseSinkIndex;
        }
        else
        {
            Q_EMIT SinkDelete(pulseSinkIndex);
        }
        break;
    }
    case PulseSinkEvent::PULSE_SINK_EVENT_ADDED:
        addSink(pulseSink);
        break;
    default:
        break;
    }
}

void AudioManager::processSinkInputEvent(PulseSinkInputEvent event, QSharedPointer<PulseSinkInput> pulseSinkInput)
{
    RETURN_IF_TRUE(m_backend->getState() != AudioState::AUDIO_STATE_READY);

    switch (event)
    {
    case PulseSinkInputEvent::PULSE_SINK_INPUT_EVENT_DELETED:
    {
        RETURN_IF_FALSE(pulseSinkInput);

        auto sinkInputIndex = pulseSinkInput->getIndex();
        if (0 == m_sinkInputs.remove(sinkInputIndex))
        {
            KLOG_WARNING(audio) << "Not found audio sink input" << sinkInputIndex;
        }
        else
        {
            Q_EMIT SinkInputDelete(sinkInputIndex);
        }
        break;
    }
    case PulseSinkInputEvent::PULSE_SINK_INPUT_EVENT_ADDED:
    {
        addSinkInput(pulseSinkInput);
        break;
    }
    default:
        break;
    }
}

void AudioManager::processSourceEvent(PulseSourceEvent event, QSharedPointer<PulseSource> pulseSource)
{
    RETURN_IF_TRUE(m_backend->getState() != AudioState::AUDIO_STATE_READY);

    switch (event)
    {
    case PulseSourceEvent::PULSE_SOURCE_EVENT_DELETED:
    {
        RETURN_IF_FALSE(pulseSource);
        auto sourceIndex = pulseSource->getIndex();
        if (0 == m_sources.remove(sourceIndex))
        {
            KLOG_WARNING(audio) << "Not found audio source" << sourceIndex;
        }
        else
        {
            Q_EMIT SourceDelete(sourceIndex);
        }
        break;
    }
    case PulseSourceEvent::PULSE_SOURCE_EVENT_ADDED:
        addSource(pulseSource);
        break;
    default:
        break;
    }
}

void AudioManager::processSourceOutputEvent(PulseSourceOutputEvent event, QSharedPointer<PulseSourceOutput> pulseSourceOutput)
{
    RETURN_IF_TRUE(m_backend->getState() != AudioState::AUDIO_STATE_READY);

    switch (event)
    {
    case PulseSourceOutputEvent::PULSE_SOURCE_OUTPUT_EVENT_DELETED:
    {
        RETURN_IF_FALSE(pulseSourceOutput);
        auto sourceOutputIndex = pulseSourceOutput->getIndex();
        if (0 == m_sourceOutputs.remove(sourceOutputIndex))
        {
            KLOG_WARNING(audio) << "Not found audio source output" << sourceOutputIndex;
        }
        else
        {
            Q_EMIT SourceOutputDelete(sourceOutputIndex);
        }
        break;
    }
    case PulseSourceOutputEvent::PULSE_SOURCE_OUTPUT_EVENT_ADDED:
        addSourceOutput(pulseSourceOutput);
        break;
    default:
        break;
    }
}

}  // namespace Kiran