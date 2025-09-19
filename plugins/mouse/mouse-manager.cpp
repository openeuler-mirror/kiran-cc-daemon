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

#include "mouse-manager.h"
#include <QDBusConnection>
#include <QDBusMessage>
#include <QGSettings>
#include <algorithm>
#include "lib/base/base.h"
#include "lib/input/input-backend.h"
#include "lib/input/input-device.h"
#include "mouse-i.h"
#include "mouseadaptor.h"

namespace Kiran
{
#define MOUSE_SCHEMA_ID "com.kylinsec.kiran.mouse"
#define MOUSE_SCHEMA_LEFT_HANDED "left-handed"
#define MOUSE_SCHEMA_MOTION_ACCELERATION "motion-acceleration"
#define MOUSE_SCHEMA_MIDDLE_EMULATION_ENABLED "middle-emulation-enabled"
#define MOUSE_SCHEMA_NATURAL_SCROLL "natural-scroll"

#define MOUSE_PROP_LEFT_HANDED "libinput Left Handed Enabled"
#define MOUSE_PROP_ACCEL_SPEED "libinput Accel Speed"
#define MOUSE_PROP_MIDDLE_EMULATION_ENABLED "libinput Middle Emulation Enabled"
#define MOUSE_PROP_NATURAL_SCROLL "libinput Natural Scrolling Enabled"

MouseManager::MouseManager() : m_leftHanded(false),
                               m_motionAcceleration(0),
                               m_middleEmulationEnabled(false),
                               m_naturalScroll(false)
{
    m_adaptor = new MouseAdaptor(this);
    m_mouseSettings = new QGSettings(MOUSE_SCHEMA_ID, "", this);
    m_inputBackend = InputBackend::getInstance();
}

MouseManager::~MouseManager()
{
}

MouseManager *MouseManager::m_instance = nullptr;
void MouseManager::globalInit()
{
    m_instance = new MouseManager();
    m_instance->init();
}

#define SEND_PROPERTY_NOTIFY(property, propertyHump)                          \
    QVariantMap changedProperties;                                            \
    changedProperties.insert(QStringLiteral(#property), get##propertyHump()); \
                                                                              \
    QDBusMessage signalMessage = QDBusMessage::createSignal(                  \
        MOUSE_OBJECT_PATH,                                                    \
        QStringLiteral("org.freedesktop.DBus.Properties"),                    \
        QStringLiteral("PropertiesChanged"));                                 \
                                                                              \
    signalMessage.setArguments({                                              \
        MOUSE_DBUS_INTERFACE_NAME,                                            \
        changedProperties,                                                    \
        QStringList(),                                                        \
    });                                                                       \
    QDBusConnection::sessionBus().send(signalMessage);

void MouseManager::setLeftHanded(bool leftHandled)
{
    RETURN_IF_TRUE(leftHandled == getLeftHanded());

    m_leftHanded = leftHandled;
    if (m_mouseSettings->get(MOUSE_SCHEMA_LEFT_HANDED).toBool() != leftHandled)
    {
        m_mouseSettings->set(MOUSE_SCHEMA_LEFT_HANDED, leftHandled);
    }
    setLeftHandedToDevices();

    SEND_PROPERTY_NOTIFY(left_handed, LeftHanded)
}

void MouseManager::setMiddleEmulationEnabled(bool middleEmulationEnabled)
{
    RETURN_IF_TRUE(middleEmulationEnabled == getMiddleEmulationEnabled());

    m_middleEmulationEnabled = middleEmulationEnabled;
    if (m_mouseSettings->get(MOUSE_SCHEMA_MIDDLE_EMULATION_ENABLED).toBool() != middleEmulationEnabled)
    {
        m_mouseSettings->set(MOUSE_SCHEMA_MIDDLE_EMULATION_ENABLED, middleEmulationEnabled);
    }
    setMiddleEmulationEnabledToDevices();

    SEND_PROPERTY_NOTIFY(middle_emulation_enabled, MiddleEmulationEnabled)
}

void MouseManager::setMotionAcceleration(double motionAcceleration)
{
    RETURN_IF_TRUE(std::fabs(motionAcceleration - getMotionAcceleration()) < EPS)

    m_motionAcceleration = motionAcceleration;
    if (std::fabs(m_mouseSettings->get(MOUSE_SCHEMA_MOTION_ACCELERATION).toDouble() - motionAcceleration) > EPS)
    {
        m_mouseSettings->set(MOUSE_SCHEMA_MOTION_ACCELERATION, motionAcceleration);
    }
    setMotionAccelerationToDevices();

    SEND_PROPERTY_NOTIFY(motion_acceleration, MotionAcceleration)
}

void MouseManager::setNaturalScroll(bool naturalScroll)
{
    RETURN_IF_TRUE(naturalScroll == getNaturalScroll());

    m_naturalScroll = naturalScroll;
    if (m_mouseSettings->get(MOUSE_SCHEMA_NATURAL_SCROLL).toBool() != naturalScroll)
    {
        m_mouseSettings->set(MOUSE_SCHEMA_NATURAL_SCROLL, naturalScroll);
    }
    setNaturalScrollToDevices();

    SEND_PROPERTY_NOTIFY(natural_scroll, NaturalScroll)
}

void MouseManager::Reset()
{
    setLeftHanded(false);
    setMotionAcceleration(0);
    setMiddleEmulationEnabled(false);
    setNaturalScroll(false);
}

void MouseManager::init()
{
    if (!m_inputBackend->isValid())
    {
        KLOG_WARNING(mouse) << "Input backend is not supported, not applying any settings.";
        return;
    }

    loadFromSettings();
    setAllPropsToDevices();

    connect(m_mouseSettings, &QGSettings::changed, this, &MouseManager::processSettingsChanged);
    connect(m_inputBackend, &InputBackend::deviceChanged, this, &MouseManager::setAllPropsToDevices);

    auto sessionConnection = QDBusConnection::sessionBus();
    if (!sessionConnection.registerService(MOUSE_DBUS_NAME))
    {
        KLOG_WARNING(audio) << "Failed to register dbus name:" << MOUSE_DBUS_NAME;
        return;
    }

    if (!sessionConnection.registerObject(MOUSE_OBJECT_PATH, MOUSE_DBUS_INTERFACE_NAME, this))
    {
        KLOG_ERROR(audio) << "Can't register object:" << sessionConnection.lastError();
        return;
    }
}

void MouseManager::loadFromSettings()
{
    m_leftHanded = m_mouseSettings->get(MOUSE_SCHEMA_LEFT_HANDED).toBool();
    m_motionAcceleration = m_mouseSettings->get(MOUSE_SCHEMA_MOTION_ACCELERATION).toDouble();
    m_middleEmulationEnabled = m_mouseSettings->get(MOUSE_SCHEMA_MIDDLE_EMULATION_ENABLED).toBool();
    m_naturalScroll = m_mouseSettings->get(MOUSE_SCHEMA_NATURAL_SCROLL).toBool();
}

void MouseManager::processSettingsChanged(const QString &key)
{
    KLOG_INFO(mouse) << "The" << key << "settings changed.";

    switch (shash(key.toLatin1().data()))
    {
    case CONNECT(MOUSE_SCHEMA_LEFT_HANDED, _hash):
        setLeftHanded(m_mouseSettings->get(key).toBool());
        break;
    case CONNECT(MOUSE_SCHEMA_MOTION_ACCELERATION, _hash):
        setMotionAcceleration(m_mouseSettings->get(key).toDouble());
        break;
    case CONNECT(MOUSE_SCHEMA_MIDDLE_EMULATION_ENABLED, _hash):
        setMiddleEmulationEnabled(m_mouseSettings->get(key).toBool());
        break;
    case CONNECT(MOUSE_SCHEMA_NATURAL_SCROLL, _hash):
        setNaturalScroll(m_mouseSettings->get(key).toBool());
        break;
    default:
        break;
    }
}

void MouseManager::setAllPropsToDevices()
{
    setLeftHandedToDevices();
    setMotionAccelerationToDevices();
    setMiddleEmulationEnabledToDevices();
    setNaturalScrollToDevices();
}

void MouseManager::setLeftHandedToDevices()
{
    setPropToDevices(MOUSE_PROP_LEFT_HANDED, QVector<bool>{getLeftHanded()});
}

void MouseManager::setMotionAccelerationToDevices()
{
    setPropToDevices(MOUSE_PROP_ACCEL_SPEED, getMotionAcceleration());
}

void MouseManager::setMiddleEmulationEnabledToDevices()
{
    setPropToDevices(MOUSE_PROP_MIDDLE_EMULATION_ENABLED, QVector<bool>{getMiddleEmulationEnabled()});
}

void MouseManager::setNaturalScrollToDevices()
{
    setPropToDevices(MOUSE_PROP_NATURAL_SCROLL, QVector<bool>{getNaturalScroll()});
}

void MouseManager::setPropToDevices(const QString &name, float value)
{
    auto inputDevices = m_inputBackend->getDevices();

    for (auto device : inputDevices)
    {
        if (device->hasProperty(name) && !device->isTouchpad())
        {
            device->setProperty(name, value);
        }
    }
}
void MouseManager::setPropToDevices(const QString &name, const QVector<bool> &values)
{
    auto inputDevices = m_inputBackend->getDevices();

    for (auto device : inputDevices)
    {
        if (device->hasProperty(name) && !device->isTouchpad())
        {
            device->setProperty(name, values);
        }
    }
}

}  // namespace Kiran