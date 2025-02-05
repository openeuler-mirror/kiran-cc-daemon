/**
 * Copyright (c) 2024 ~ 2025 KylinSec Co., Ltd.
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

#include "keys-sound.h"
#include <QDBusPendingCallWatcher>
#include <QDBusServiceWatcher>
#include <QGSettings>
#include <QKeySequence>
#include "audio-i.h"
#include "audio_dbus_proxy.h"
#include "audio_device_dbus_proxy.h"
#include "keybinding-i.h"
#include "lib/base/base.h"

namespace Kiran
{

#define ACTION_NAME_VOLUME_MUTE "volume-mute"
#define ACTION_NAME_VOLUME_DOWN "volume-down"
#define ACTION_NAME_VOLUME_UP "volume-up"
#define ACTION_NAME_MIC_VOLUME_MUTE "mic-volume-mute"
#define ACTION_NAME_MIC_VOLUME_DOWN "mic-volume-down"
#define ACTION_NAME_MIC_VOLUME_UP "mic-volume-up"
#define ACTION_NAME_MEDIA "media"
#define ACTION_NAME_PLAY "play"
#define ACTION_NAME_PAUSE "pause"
#define ACTION_NAME_STOP "stop"
#define ACTION_NAME_PREVIOUS "previous"
#define ACTION_NAME_NEXT "next"

// 默认音量步进
#define VOLUME_STEP_DEFAULT 6

KeysSound::KeysSound() : KeysComponent("Sound", tr("Sound")),
                         m_audioProxy(nullptr),
                         m_audioSinkDeviceProxy(nullptr),
                         m_audioSourceDeviceProxy(nullptr)
{
    if (QDBusConnection::sessionBus().interface()->isServiceRegistered(AUDIO_DBUS_NAME))
    {
        initAudioProxy();
    }
    else
    {
        m_audioServiceWatcher = new QDBusServiceWatcher(AUDIO_DBUS_NAME,
                                                        QDBusConnection::sessionBus(),
                                                        QDBusServiceWatcher::WatchForRegistration, this);

        connect(m_audioServiceWatcher, &QDBusServiceWatcher::serviceRegistered, this, [this]()
                { this->initAudioProxy(); });
    }
}

void KeysSound::init()
{
    registerShortCut(Qt::Key_VolumeMute, ACTION_NAME_VOLUME_MUTE, tr("Volume mute"));
    registerShortCut(Qt::Key_VolumeDown, ACTION_NAME_VOLUME_DOWN, tr("Volume down"));
    registerShortCut(Qt::Key_VolumeUp, ACTION_NAME_VOLUME_UP, tr("Volume up"));
    registerShortCut(Qt::Key_MicMute, ACTION_NAME_MIC_VOLUME_MUTE, tr("Microphone mute"));
    registerShortCut(Qt::Key_MicVolumeDown, ACTION_NAME_MIC_VOLUME_DOWN, tr("Microphone volume down"));
    registerShortCut(Qt::Key_MicVolumeUp, ACTION_NAME_MIC_VOLUME_UP, tr("Microphone volume up"));
    registerShortCut(Qt::Key_MediaPlay, ACTION_NAME_MEDIA, tr("Launch media player"));
    registerShortCut(Qt::Key_MediaPlay, ACTION_NAME_PLAY, tr("Play (or play/pause)"));
    registerShortCut(Qt::Key_MediaPause, ACTION_NAME_PAUSE, tr("Pause playback"));
    registerShortCut(Qt::Key_MediaStop, ACTION_NAME_STOP, tr("Stop playback"));
    registerShortCut(Qt::Key_MediaPrevious, ACTION_NAME_PREVIOUS, tr("Previous track"));
    registerShortCut(Qt::Key_MediaNext, ACTION_NAME_NEXT, tr("Next track"));
}

void KeysSound::initAudioProxy()
{
    if (m_audioProxy)
    {
        delete m_audioProxy;
        m_audioProxy = nullptr;
    }

    m_audioProxy = new AudioProxy(AUDIO_DBUS_NAME, AUDIO_OBJECT_PATH, QDBusConnection::sessionBus(), this);

    updateAudioDevice();
    // TODO: 测试不监听state属性变化有没有问题
    connect(m_audioProxy, &AudioProxy::DefaultSinkChange, this, &KeysSound::updateAudioSinkDevice);
    connect(m_audioProxy, &AudioProxy::DefaultSourceChange, this, &KeysSound::updateAudioSourceDevice);
}

void KeysSound::muteDevice(AudioDeviceProxy *deviceProxy)
{
    RETURN_IF_FALSE(deviceProxy);

    bool newMuted = !deviceProxy->mute();

    auto mutedWatcher = new QDBusPendingCallWatcher(deviceProxy->SetMute(newMuted));
    connect(mutedWatcher,
            &QDBusPendingCallWatcher::finished,
            this,
            [this, newMuted](QDBusPendingCallWatcher *mutedWatcher)
            {
                QDBusPendingReply<> mutedReply = *mutedWatcher;
                mutedWatcher->deleteLater();
                if (newMuted)
                {
                    // TODO：显示图标
                }
            });
}

void KeysSound::downDeviceVolume(AudioDeviceProxy *deviceProxy)
{
    RETURN_IF_FALSE(deviceProxy);

    double volumeLast = deviceProxy->volume();
    double volume = volumeLast;

    uint32_t volumeStep = m_settings->get(KEYS_SCHEMA_VOLUME_STEP).toInt();
    if (volumeStep == 0 || volumeStep > 100)
    {
        volumeStep = VOLUME_STEP_DEFAULT;
    }

    double volumeStepAbs = volumeStep / 100.0;

    if (volume < volumeStepAbs)
    {
        muteDevice(deviceProxy);
        return;
    }

    volume = qBound(0.0, volume - volumeStepAbs, 1.0);
    KLOG_INFO(keybinding) << "Down volume from" << volumeLast << "to" << volume << "for device" << deviceProxy->name();
    setDeviceVolume(deviceProxy, volume);
}

void KeysSound::upDeviceVolume(AudioDeviceProxy *deviceProxy)
{
    RETURN_IF_FALSE(deviceProxy);

    double volumeLast = deviceProxy->volume();
    double volume = volumeLast;

    uint32_t volumeStep = m_settings->get(KEYS_SCHEMA_VOLUME_STEP).toInt();
    if (volumeStep == 0 || volumeStep > 100)
    {
        volumeStep = VOLUME_STEP_DEFAULT;
    }

    double volumeStepAbs = volumeStep / 100.0;

    volume = qBound(0.0, volume + volumeStepAbs, 1.0);
    KLOG_INFO(keybinding) << "Up volume from" << volumeLast << "to" << volume << "for device" << deviceProxy->name();
    setDeviceVolume(deviceProxy, volume);
}

void KeysSound::setDeviceVolume(AudioDeviceProxy *deviceProxy, double volume)
{
    auto volumeWatcher = new QDBusPendingCallWatcher(deviceProxy->SetVolume(volume));
    connect(volumeWatcher,
            &QDBusPendingCallWatcher::finished,
            this,
            [this](QDBusPendingCallWatcher *volumeWatcher)
            {
                QDBusPendingReply<> volumeReply = *volumeWatcher;
                volumeWatcher->deleteLater();
                // TODO：显示图标
            });
}

void KeysSound::updateAudioDevice()
{
    updateAudioSinkDevice();
    updateAudioSourceDevice();
}

void KeysSound::updateAudioSinkDevice()
{
    auto defaultSinkWatcher = new QDBusPendingCallWatcher(m_audioProxy->GetDefaultSink());
    connect(defaultSinkWatcher,
            &QDBusPendingCallWatcher::finished,
            this,
            [this](QDBusPendingCallWatcher *defaultSinkWatcher)
            {
                QDBusPendingReply<QDBusObjectPath> defaultSinkReply = *defaultSinkWatcher;
                defaultSinkWatcher->deleteLater();
                this->m_audioSinkDeviceProxy = new AudioDeviceProxy(AUDIO_DBUS_NAME, defaultSinkReply.value().path(), QDBusConnection::sessionBus(), this);
            });
}

void KeysSound::updateAudioSourceDevice()
{
    auto defaultSourceWatcher = new QDBusPendingCallWatcher(m_audioProxy->GetDefaultSource());
    connect(defaultSourceWatcher,
            &QDBusPendingCallWatcher::finished,
            this,
            [this](QDBusPendingCallWatcher *defaultSourceWatcher)
            {
                QDBusPendingReply<QDBusObjectPath> defaultSourceReply = *defaultSourceWatcher;
                defaultSourceWatcher->deleteLater();
                this->m_audioSourceDeviceProxy = new AudioDeviceProxy(AUDIO_DBUS_NAME, defaultSourceReply.value().path(), QDBusConnection::sessionBus(), this);
            });
}

void KeysSound::triggerShortCut(const QString &name)
{
    switch (shash(name.toLatin1().data()))
    {
    case CONNECT(ACTION_NAME_VOLUME_MUTE, _hash):
        muteDevice(m_audioSinkDeviceProxy);
        break;
    case CONNECT(ACTION_NAME_VOLUME_DOWN, _hash):
        downDeviceVolume(m_audioSinkDeviceProxy);
        break;
    case CONNECT(ACTION_NAME_VOLUME_UP, _hash):
        upDeviceVolume(m_audioSinkDeviceProxy);
        break;
    case CONNECT(ACTION_NAME_MIC_VOLUME_MUTE, _hash):
        muteDevice(m_audioSourceDeviceProxy);
        break;
    case CONNECT(ACTION_NAME_MIC_VOLUME_DOWN, _hash):
        downDeviceVolume(m_audioSourceDeviceProxy);
        break;
    case CONNECT(ACTION_NAME_MIC_VOLUME_UP, _hash):
        upDeviceVolume(m_audioSourceDeviceProxy);
        break;
    case CONNECT(ACTION_NAME_MEDIA, _hash):
    case CONNECT(ACTION_NAME_PLAY, _hash):
    case CONNECT(ACTION_NAME_PAUSE, _hash):
    case CONNECT(ACTION_NAME_STOP, _hash):
    case CONNECT(ACTION_NAME_PREVIOUS, _hash):
    case CONNECT(ACTION_NAME_NEXT, _hash):
        KLOG_WARNING(keybinding) << "Not support action: " << name;
        break;
    default:
        break;
    }
}

}  // namespace Kiran
