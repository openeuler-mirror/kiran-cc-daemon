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

#include "audio-stream.h"
#include "audio-utils.h"
#include "lib/base/base.h"
#include "pulse/pulse-stream.h"
#include "streamadaptor.h"

namespace Kiran
{
AudioStream::AudioStream(QSharedPointer<PulseStream> stream) : m_stream(stream)
{
    m_adaptor = new StreamAdaptor(this);

    m_mute = m_stream->getMute();
    m_volume = AudioUtils::volumeAbsolute2range(m_stream->getVolume(),
                                                m_stream->getMinVolume(),
                                                m_stream->getMaxVolume());

    connect(qSharedPointerCast<PulseNode>(m_stream).data(), &PulseNode::nodeInfoChanged, this, &AudioStream::processNodeInfoChanged);
}

AudioStream::~AudioStream()
{
    dbusUnregister();
}

bool AudioStream::init(const QString &objectPathPrefix)
{
    RETURN_VAL_IF_FALSE(m_stream, false);

    m_objectPath = QString("%1%2").arg(objectPathPrefix).arg(m_stream->getIndex());
    return dbusRegister();
}

#define SEND_PROPERTY_NOTIFY(property, propertyHump)                                \
    QVariantMap changedProperties;                                                  \
    changedProperties.insert(QStringLiteral(#property), this->get##propertyHump()); \
                                                                                    \
    QDBusMessage message = QDBusMessage::createSignal(                              \
        this->m_objectPath,                                                         \
        QStringLiteral("org.freedesktop.DBus.Properties"),                          \
        QStringLiteral("PropertiesChanged"));                                       \
                                                                                    \
    message.setArguments({                                                          \
        QStringLiteral(AUDIO_STREAM_DBUS_INTERFACE_NAME),                           \
        changedProperties,                                                          \
        QStringList(),                                                              \
    });                                                                             \
                                                                                    \
    QDBusConnection::sessionBus().send(message);

void AudioStream::setMute(bool mute)
{
    if (m_mute != mute)
    {
        m_mute = mute;
        SEND_PROPERTY_NOTIFY(mute, Mute)
    }
}
void AudioStream::setVolume(double volume)
{
    if (std::fabs(m_volume - volume) > EPS)
    {
        m_volume = volume;
        SEND_PROPERTY_NOTIFY(volume, Volume)
    }
}

uint AudioStream::getIndex() const
{
    return m_stream->getIndex();
}

QString AudioStream::getName() const
{
    return m_stream->getName();
}
uint AudioStream::getState() const
{
    return m_stream->getFlags();
}

QString AudioStream::GetProperty(const QString &key)
{
    return m_stream->getProperty(key);
}

void AudioStream::SetMute(bool mute)
{
    if (!m_stream->setMute(mute))
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_AUDIO_STREAM_SET_MUTE_FAILED);
    }
}

void AudioStream::SetVolume(double volume)
{
    if (volume < 0 || volume > 1.0 + EPS)
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_AUDIO_STREAM_VOLUME_RANGE_INVLAID);
    }

    auto volumeAbsolute = AudioUtils::volumeRange2absolute(volume,
                                                           m_stream->getMinVolume(),
                                                           m_stream->getMaxVolume());

    if (!m_stream->setVolume(volumeAbsolute))
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_AUDIO_STREAM_SET_VOLUME_FAILED);
    }

    // 如果音量大于0，则取消静音
    if (volume > EPS)
    {
        m_stream->setMute(false);
    }
}

bool AudioStream::dbusRegister()
{
    KLOG_INFO(audio) << "Register object path" << m_objectPath;

    RETURN_VAL_IF_FALSE(m_stream, false);

    auto sessionConnection = QDBusConnection::sessionBus();
    if (!sessionConnection.registerObject(m_objectPath, AUDIO_STREAM_DBUS_INTERFACE_NAME, this))
    {
        KLOG_ERROR(audio) << "Can't register object:" << sessionConnection.lastError();
        return false;
    }

    return true;
}

void AudioStream::dbusUnregister()
{
    auto sessionConnection = QDBusConnection::sessionBus();
    sessionConnection.unregisterObject(m_objectPath);
}

void AudioStream::processNodeInfoChanged(int32_t field)
{
    switch (field)
    {
    case PulseNodeField::PULSE_NODE_FIELD_MUTE:
        setMute(m_stream->getMute());
        break;
    case PulseNodeField::PULSE_NODE_FIELD_VOLUME:
    {
        auto volume = AudioUtils::volumeAbsolute2range(m_stream->getVolume(),
                                                       m_stream->getMinVolume(),
                                                       m_stream->getMaxVolume());

        setVolume(volume);
        break;
    }
    default:
        break;
    }
}

}  // namespace Kiran