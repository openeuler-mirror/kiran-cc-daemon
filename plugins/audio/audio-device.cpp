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

#include "audio-device.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include "audio-i.h"
#include "audio-utils.h"
#include "deviceadaptor.h"
#include "lib/base/base.h"
#include "pulse/pulse-device.h"

namespace Kiran
{
AudioDevice::AudioDevice(QSharedPointer<PulseDevice> device) : m_device(device)
{
    m_adaptor = new DeviceAdaptor(this);

    m_mute = m_device->getMute();
    m_volume = AudioUtils::volumeAbsolute2range(m_device->getVolume(),
                                                m_device->getMinVolume(),
                                                m_device->getMaxVolume());
    m_balance = m_device->getBalance();
    m_fade = m_device->getFade();
    m_activePort = m_device->getActivePort();

    connect(m_device.data(), &PulseDevice::nodeInfoChanged, this, &AudioDevice::processNodeInfoChanged);
    connect(m_device.data(), &PulseDevice::activePortChanged, this, &AudioDevice::setActivePort);
}

AudioDevice::~AudioDevice()
{
    dbusUnregister();
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
        QStringLiteral(AUDIO_DEVICE_DBUS_INTERFACE_NAME),                           \
        changedProperties,                                                          \
        QStringList(),                                                              \
    });                                                                             \
                                                                                    \
    QDBusConnection::sessionBus().send(message);

bool AudioDevice::init(const QString &objectPathPrefix)
{
    RETURN_VAL_IF_FALSE(m_device, false);

    m_objectPath = QString("%1%2").arg(objectPathPrefix).arg(m_device->getIndex());
    return dbusRegister();
}

double AudioDevice::getBaseVolume() const
{
    auto volume = m_device->getBaseVolume();
    return AudioUtils::volumeAbsolute2range(volume, m_device->getMinVolume(), m_device->getMaxVolume());
}

uint AudioDevice::getCardIndex() const
{
    return m_device->getCardIndex();
}

uint AudioDevice::getIndex() const
{
    return m_device->getIndex();
}

QString AudioDevice::getName() const
{
    return m_device->getName();
}

uint AudioDevice::getState() const
{
    return m_device->getFlags();
}

void AudioDevice::setActivePort(const QString &activePort)
{
    if (m_activePort != activePort)
    {
        m_activePort = activePort;
        SEND_PROPERTY_NOTIFY(active_port, ActivePort)
    }
}

void AudioDevice::setBalance(double balance)
{
    if (std::fabs(m_balance - balance) > EPS)
    {
        m_balance = balance;
        SEND_PROPERTY_NOTIFY(balance, Balance)
    }
}

void AudioDevice::setFade(double fade)
{
    if (std::fabs(m_fade - fade) > EPS)
    {
        m_fade = fade;
        SEND_PROPERTY_NOTIFY(fade, Fade);
    }
}

void AudioDevice::setMute(bool mute)
{
    if (std::fabs(m_mute - mute) > EPS)
    {
        m_mute = mute;
        SEND_PROPERTY_NOTIFY(mute, Mute)
    }
}

void AudioDevice::setVolume(double volume)
{
    if (std::fabs(m_volume - volume) > EPS)
    {
        m_volume = volume;
        SEND_PROPERTY_NOTIFY(volume, Volume)
    }
}

QString AudioDevice::GetPorts()
{
    QJsonArray jsonPorts;

    for (auto port : m_device->getPorts())
    {
        QJsonObject jsonPort;
        jsonPort["name"] = port->getName();
        jsonPort["description"] = port->getDescription();
        jsonPort["priority"] = int(port->getPriority());
        jsonPort["available"] = port->getAvailable();
        jsonPorts.append(jsonPort);
    }
    return QJsonDocument(jsonPorts).toJson(QJsonDocument::Compact);
}

QString AudioDevice::GetProperty(const QString &key)
{
    return m_device->getProperty(key);
}

void AudioDevice::SetActivePort(const QString &name)
{
    if (!m_device->setActivePort(name))
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_AUDIO_SET_SINK_ACTIVE_PORT_FAILED);
    }
}

void AudioDevice::SetBalance(double balance)
{
    if (balance < -1 || balance > 1)
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_AUDIO_DEVICE_BALANCE_RANGE_INVLAID);
    }

    if (!m_device->setBalance(balance))
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_AUDIO_DEVICE_SET_BALANCE_FAILED);
    }
}

void AudioDevice::SetFade(double fade)
{
    if (fade < -1 || fade > 1)
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_AUDIO_DEVICE_FADE_RANGE_INVLAID);
    }

    if (!m_device->setFade(fade))
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_AUDIO_DEVICE_SET_FADE_FAILED);
    }
}

void AudioDevice::SetMute(bool mute)
{
    if (!m_device->setMute(mute))
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_AUDIO_DEVICE_SET_MUTE_FAILED);
    }
}

void AudioDevice::SetVolume(double volume)
{
    if (volume < 0 || volume > 1.0 + EPS)
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_AUDIO_DEVICE_VOLUME_RANGE_INVLAID);
    }

    auto volumeAbsolute = AudioUtils::volumeRange2absolute(volume,
                                                           m_device->getMinVolume(),
                                                           m_device->getMaxVolume());

    if (!m_device->setVolume(volumeAbsolute))
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_AUDIO_DEVICE_SET_VOLUME_FAILED);
    }

    // 如果音量大于0，则取消静音
    if (volume > EPS)
    {
        m_device->setMute(false);
    }
}

bool AudioDevice::dbusRegister()
{
    KLOG_INFO(audio) << "Register object path" << m_objectPath;

    RETURN_VAL_IF_FALSE(m_device, false);

    auto sessionConnection = QDBusConnection::sessionBus();
    if (!sessionConnection.registerObject(m_objectPath, AUDIO_DEVICE_DBUS_INTERFACE_NAME, this))
    {
        KLOG_ERROR(audio) << "Can't register object:" << sessionConnection.lastError();
        return false;
    }

    return true;
}

void AudioDevice::dbusUnregister()
{
    auto sessionConnection = QDBusConnection::sessionBus();
    sessionConnection.unregisterObject(m_objectPath);
}

void AudioDevice::processNodeInfoChanged(int32_t field)
{
    switch (field)
    {
    case PulseNodeField::PULSE_NODE_FIELD_BALANCE:
        setBalance(m_device->getBalance());
        break;
    case PulseNodeField::PULSE_NODE_FIELD_FADE:
        setFade(m_device->getFade());
        break;
    case PulseNodeField::PULSE_NODE_FIELD_MUTE:
        setMute(m_device->getMute());
        break;
    case PulseNodeField::PULSE_NODE_FIELD_VOLUME:
    {
        auto volume = AudioUtils::volumeAbsolute2range(m_device->getVolume(),
                                                       m_device->getMinVolume(),
                                                       m_device->getMaxVolume());
        setVolume(volume);
        break;
    }
    default:
        break;
    }
}

}  // namespace Kiran