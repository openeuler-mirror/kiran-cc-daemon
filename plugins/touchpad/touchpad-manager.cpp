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

#include "touchpad-manager.h"
#include <QDBusConnection>
#include <QDBusMessage>
#include <QGSettings>
#include <algorithm>
#include "lib/base/base.h"
#include "lib/input/input-backend.h"
#include "lib/input/input-device.h"
#include "touchpad-i.h"
#include "touchpadadaptor.h"

namespace Kiran
{
#define X_HASH(X) CONNECT(X, _hash)

#define TOUCHPAD_SCHEMA_ID "com.kylinsec.kiran.touchpad"
#define TOUCHPAD_SCHEMA_LEFT_HANDED "left-handed"
#define TOUCHPAD_SCHEMA_DISABLE_WHILE_TYPING "disable-while-typing"
#define TOUCHPAD_SCHEMA_TAP_TO_CLICK "tap-to-click"
#define TOUCHPAD_SCHEMA_CLICK_METHOD "click-method"
#define TOUCHPAD_SCHEMA_SCROLL_METHOD "scroll-method"
#define TOUCHPAD_SCHEMA_NATURAL_SCROLL "natural-scroll"
#define TOUCHPAD_SCHEMA_TOUCHPAD_ENABLED "touchpad-enabled"
#define TOUCHPAD_SCHEMA_MOTION_ACCELERATION "motion-acceleration"

#define TOUCHPAD_PROP_LEFT_HANDED "libinput Left Handed Enabled"
#define TOUCHPAD_PROP_DISABLE_WHILE_TYPING "libinput Disable While Typing Enabled"
#define TOUCHPAD_PROP_TAPPING_ENABLED "libinput Tapping Enabled"
#define TOUCHPAD_PROP_CLICK_METHOD "libinput Click Method Enabled"
#define TOUCHPAD_PROP_SCROLL_METHOD "libinput Scroll Method Enabled"
#define TOUCHPAD_PROP_NATURAL_SCROLL "libinput Natural Scrolling Enabled"
#define TOUCHPAD_PROP_DEVICE_ENABLED "Device Enabled"
#define TOUCHPAD_PROP_ACCEL_SPEED "libinput Accel Speed"

TouchPadManager::TouchPadManager() : m_hasTouchpad(false),
                                     m_leftHanded(false),
                                     m_disableWhileTyping(false),
                                     m_tapToClick(true),
                                     m_clickMethod(0),
                                     m_scrollMethod(0),
                                     m_naturalScroll(false),
                                     m_touchpadEnabled(true),
                                     m_motionAcceleration(0),
                                     m_disableWhileTypingSupport(false),
                                     m_tapToClickSupport(false),
                                     m_clickMethodSupport(false)
{
    m_adaptor = new TouchPadAdaptor(this);
    m_touchpadSettings = new QGSettings(TOUCHPAD_SCHEMA_ID, "", this);
    m_inputBackend = InputBackend::getDefault();
}

TouchPadManager::~TouchPadManager()
{
}

TouchPadManager *TouchPadManager::m_instance = nullptr;
void TouchPadManager::globalInit()
{
    m_instance = new TouchPadManager();
    m_instance->init();
}

#define SEND_PROPERTY_NOTIFY(property, propertyHump)                          \
    QVariantMap changedProperties;                                            \
    changedProperties.insert(QStringLiteral(#property), get##propertyHump()); \
                                                                              \
    QDBusMessage signalMessage = QDBusMessage::createSignal(                  \
        TOUCHPAD_OBJECT_PATH,                                                 \
        QStringLiteral("org.freedesktop.DBus.Properties"),                    \
        QStringLiteral("PropertiesChanged"));                                 \
                                                                              \
    signalMessage.setArguments({                                              \
        TOUCHPAD_DBUS_INTERFACE_NAME,                                         \
        changedProperties,                                                    \
        QStringList(),                                                        \
    });                                                                       \
    QDBusConnection::sessionBus().send(signalMessage);

void TouchPadManager::setClickMethod(int clickMethod)
{
    RETURN_IF_TRUE(clickMethod == getClickMethod());

    if (m_touchpadSettings->get(TOUCHPAD_SCHEMA_CLICK_METHOD).toInt() != clickMethod)
    {
        m_touchpadSettings->set(TOUCHPAD_SCHEMA_CLICK_METHOD, clickMethod);
    }
    setClickMethodToDevices();

    SEND_PROPERTY_NOTIFY(click_method, ClickMethod);
}

void TouchPadManager::setClickMethodSupport(bool)
{
    // 不支持设置该属性
}

void TouchPadManager::setDisableWhileTyping(bool disableWhileTyping)
{
    RETURN_IF_TRUE(disableWhileTyping == getDisableWhileTyping());

    if (m_touchpadSettings->get(TOUCHPAD_SCHEMA_DISABLE_WHILE_TYPING).toBool() != disableWhileTyping)
    {
        m_touchpadSettings->set(TOUCHPAD_SCHEMA_DISABLE_WHILE_TYPING, disableWhileTyping);
    }
    setDisableWhileTypingToDevices();

    SEND_PROPERTY_NOTIFY(disable_while_typing, DisableWhileTyping)
}

void TouchPadManager::setDisableWhileTypingSupport(bool disableWhileTypingSupport)
{
    // 不支持设置该属性
}

void TouchPadManager::setHasTouchpad(bool hasTouchpad)
{
    // 不支持设置该属性
}

void TouchPadManager::setLeftHanded(bool leftHanded)
{
    RETURN_IF_TRUE(leftHanded == getLeftHanded());

    if (m_touchpadSettings->get(TOUCHPAD_SCHEMA_LEFT_HANDED).toBool() != leftHanded)
    {
        m_touchpadSettings->set(TOUCHPAD_SCHEMA_LEFT_HANDED, leftHanded);
    }
    setLeftHandedToDevices();

    SEND_PROPERTY_NOTIFY(left_handed, LeftHanded);
}

void TouchPadManager::setMotionAcceleration(double motionAcceleration)
{
    RETURN_IF_TRUE(std::fabs(motionAcceleration - getMotionAcceleration()) < EPS);

    if (std::fabs(m_touchpadSettings->get(TOUCHPAD_SCHEMA_MOTION_ACCELERATION).toDouble() - motionAcceleration) > EPS)
    {
        m_touchpadSettings->set(TOUCHPAD_SCHEMA_MOTION_ACCELERATION, motionAcceleration);
    }
    setMotionAccelerationToDevices();

    SEND_PROPERTY_NOTIFY(motion_acceleration, MotionAcceleration);
}

void TouchPadManager::setNaturalScroll(bool naturalScroll)
{
    RETURN_IF_TRUE(naturalScroll == getNaturalScroll());

    if (m_touchpadSettings->get(TOUCHPAD_SCHEMA_NATURAL_SCROLL).toBool() != naturalScroll)
    {
        m_touchpadSettings->set(TOUCHPAD_SCHEMA_NATURAL_SCROLL, naturalScroll);
    }
    setNaturalScrollToDevices();

    SEND_PROPERTY_NOTIFY(natural_scroll, NaturalScroll);
}

void TouchPadManager::setScrollMethod(int scrollMethod)
{
    RETURN_IF_TRUE(scrollMethod == getScrollMethod());

    if (m_touchpadSettings->get(TOUCHPAD_SCHEMA_SCROLL_METHOD).toInt() != scrollMethod)
    {
        m_touchpadSettings->set(TOUCHPAD_SCHEMA_SCROLL_METHOD, scrollMethod);
    }
    setScrollMethodToDevices();

    SEND_PROPERTY_NOTIFY(scroll_method, ScrollMethod);
}

void TouchPadManager::setTapToClick(bool tapToClick)
{
    RETURN_IF_TRUE(tapToClick == getTapToClick());

    if (m_touchpadSettings->get(TOUCHPAD_SCHEMA_TAP_TO_CLICK).toBool() != tapToClick)
    {
        m_touchpadSettings->set(TOUCHPAD_SCHEMA_TAP_TO_CLICK, tapToClick);
    }
    setTapToClickToDevices();

    SEND_PROPERTY_NOTIFY(tap_to_click, TapToClick);
}

void TouchPadManager::setTapToClickSupport(bool tapToClickSupport)
{
    // 不支持设置该属性
}

void TouchPadManager::setTouchpadEnabled(bool touchpadEnabled)
{
    RETURN_IF_TRUE(touchpadEnabled == getTouchpadEnabled());

    if (m_touchpadSettings->get(TOUCHPAD_SCHEMA_TOUCHPAD_ENABLED).toBool() != touchpadEnabled)
    {
        m_touchpadSettings->set(TOUCHPAD_SCHEMA_TOUCHPAD_ENABLED, touchpadEnabled);
    }
    setTouchpadEnabledToDevices();

    SEND_PROPERTY_NOTIFY(touchpad_enabled, TouchpadEnabled);
}

void TouchPadManager::Reset()
{
    setLeftHanded(false);
    setDisableWhileTyping(false);
    setTapToClick(true);
    setClickMethod(0);
    setScrollMethod(0);
    setNaturalScroll(false);
    setTouchpadEnabled(true);
    setMotionAcceleration(0);
}

void TouchPadManager::init()
{
    if (!m_inputBackend->isValid())
    {
        KLOG_WARNING(touchpad) << "Input backend is not supported, not applying any settings.";
        return;
    }

    initProperties();

    setAllPropsToDevices();

    connect(m_touchpadSettings, &QGSettings::changed, this, &TouchPadManager::processSettingsChanged);

    auto sessionConnection = QDBusConnection::sessionBus();
    if (!sessionConnection.registerService(TOUCHPAD_DBUS_NAME))
    {
        KLOG_WARNING(audio) << "Failed to register dbus name: " << TOUCHPAD_DBUS_NAME;
        return;
    }

    if (!sessionConnection.registerObject(TOUCHPAD_OBJECT_PATH, TOUCHPAD_DBUS_INTERFACE_NAME, this))
    {
        KLOG_ERROR(audio) << "Can't register object:" << sessionConnection.lastError();
        return;
    }
}

void TouchPadManager::initProperties()
{
    auto inputDevices = m_inputBackend->getDevices();

    for (auto inputDevice : inputDevices)
    {
        CONTINUE_IF_TRUE(!inputDevice->isTouchpad());

        m_hasTouchpad = true;
        if (inputDevice->hasProperty(TOUCHPAD_PROP_DISABLE_WHILE_TYPING))
        {
            m_disableWhileTypingSupport = true;
        }

        if (inputDevice->hasProperty(TOUCHPAD_PROP_TAPPING_ENABLED))
        {
            m_tapToClickSupport = true;
        }

        if (inputDevice->hasProperty(TOUCHPAD_PROP_CLICK_METHOD))
        {
            m_clickMethodSupport = true;
        }
    }

    m_leftHanded = m_touchpadSettings->get(TOUCHPAD_SCHEMA_LEFT_HANDED).toBool();
    m_disableWhileTyping = m_touchpadSettings->get(TOUCHPAD_SCHEMA_DISABLE_WHILE_TYPING).toBool();
    m_tapToClick = m_touchpadSettings->get(TOUCHPAD_SCHEMA_TAP_TO_CLICK).toBool();
    m_clickMethod = m_touchpadSettings->get(TOUCHPAD_SCHEMA_CLICK_METHOD).toInt();
    m_scrollMethod = m_touchpadSettings->get(TOUCHPAD_SCHEMA_SCROLL_METHOD).toInt();
    m_naturalScroll = m_touchpadSettings->get(TOUCHPAD_SCHEMA_NATURAL_SCROLL).toBool();
    m_touchpadEnabled = m_touchpadSettings->get(TOUCHPAD_SCHEMA_TOUCHPAD_ENABLED).toBool();
    m_motionAcceleration = m_touchpadSettings->get(TOUCHPAD_SCHEMA_MOTION_ACCELERATION).toDouble();
}

void TouchPadManager::processSettingsChanged(const QString &key)
{
    KLOG_INFO(touchpad) << "The" << key << "settings changed.";

    switch (shash(key.toLatin1().data()))
    {
    case CONNECT(TOUCHPAD_SCHEMA_LEFT_HANDED, _hash):
        setLeftHanded(m_touchpadSettings->get(key).toBool());
        break;
    case CONNECT(TOUCHPAD_SCHEMA_DISABLE_WHILE_TYPING, _hash):
        setDisableWhileTyping(m_touchpadSettings->get(key).toBool());
        break;
    case CONNECT(TOUCHPAD_SCHEMA_TAP_TO_CLICK, _hash):
        setTapToClick(m_touchpadSettings->get(key).toBool());
        break;
    case CONNECT(TOUCHPAD_SCHEMA_CLICK_METHOD, _hash):
        setClickMethod(m_touchpadSettings->get(key).toInt());
        break;
    case CONNECT(TOUCHPAD_SCHEMA_SCROLL_METHOD, _hash):
        setScrollMethod(m_touchpadSettings->get(key).toInt());
        break;
    case CONNECT(TOUCHPAD_SCHEMA_NATURAL_SCROLL, _hash):
        setNaturalScroll(m_touchpadSettings->get(key).toBool());
        break;
    case CONNECT(TOUCHPAD_SCHEMA_TOUCHPAD_ENABLED, _hash):
        setTouchpadEnabled(m_touchpadSettings->get(key).toBool());
        break;
    case CONNECT(TOUCHPAD_SCHEMA_MOTION_ACCELERATION, _hash):
        setMotionAcceleration(m_touchpadSettings->get(key).toDouble());
        break;
    default:
        break;
    }
}

void TouchPadManager::setAllPropsToDevices()
{
    setLeftHandedToDevices();
    setDisableWhileTypingToDevices();
    setTapToClickToDevices();
    setClickMethodToDevices();
    setScrollMethodToDevices();
    setNaturalScrollToDevices();
    setTouchpadEnabledToDevices();
    setMotionAccelerationToDevices();
}

void TouchPadManager::setLeftHandedToDevices()
{
    setPropToDevices(TOUCHPAD_PROP_LEFT_HANDED, QVector<bool>{m_leftHanded});
}

void TouchPadManager::setDisableWhileTypingToDevices()
{
    setPropToDevices(TOUCHPAD_PROP_DISABLE_WHILE_TYPING, QVector<bool>{m_disableWhileTyping});
}

void TouchPadManager::setTapToClickToDevices()
{
    setPropToDevices(TOUCHPAD_PROP_TAPPING_ENABLED, QVector<bool>{m_tapToClick});
}

void TouchPadManager::setClickMethodToDevices()
{
    switch (m_clickMethod)
    {
    case int32_t(TouchPadClickMethod::TOUCHPAD_CLICK_METHOD_BUTTON_AREAS):
        setPropToDevices(TOUCHPAD_PROP_CLICK_METHOD, QVector<bool>{true, false});
        break;
    case int32_t(TouchPadClickMethod::TOUCHPAD_CLICK_METHOD_CLICK_FINGER):
        setPropToDevices(TOUCHPAD_PROP_CLICK_METHOD, QVector<bool>{false, true});
        break;
    default:
        KLOG_WARNING(touchpad) << "Unknow click methods" << m_clickMethod;
        break;
    }
}

void TouchPadManager::setScrollMethodToDevices()
{
    switch (m_scrollMethod)
    {
    case int32_t(TouchPadScrollMethod::TOUCHPAD_SCROLL_METHOD_TWO_FINGER):
        setPropToDevices(TOUCHPAD_PROP_SCROLL_METHOD, QVector<bool>{true, false, false});
        break;
    case int32_t(TouchPadScrollMethod::TOUCHPAD_SCROLL_METHOD_EDGE):
        setPropToDevices(TOUCHPAD_PROP_SCROLL_METHOD, QVector<bool>{false, true, false});
        break;
    case int32_t(TouchPadScrollMethod::TOUCHPAD_SCROLL_METHOD_BUTTON):
        setPropToDevices(TOUCHPAD_PROP_SCROLL_METHOD, QVector<bool>{false, false, true});
        break;
    default:
        KLOG_WARNING(touchpad) << "Unknow scroll methods" << m_scrollMethod;
        break;
    }
}

void TouchPadManager::setNaturalScrollToDevices()
{
    setPropToDevices(TOUCHPAD_PROP_NATURAL_SCROLL, QVector<bool>{m_naturalScroll});
}

void TouchPadManager::setTouchpadEnabledToDevices()
{
    setPropToDevices(TOUCHPAD_PROP_DEVICE_ENABLED, QVector<bool>{m_touchpadEnabled});
}

void TouchPadManager::setMotionAccelerationToDevices()
{
    setPropToDevices(TOUCHPAD_PROP_ACCEL_SPEED, (float)m_motionAcceleration);
}

void TouchPadManager::setPropToDevices(const QString &name, float value)
{
    auto inputDevices = m_inputBackend->getDevices();

    for (auto device : inputDevices)
    {
        if (device->hasProperty(name) && device->isTouchpad())
        {
            device->setProperty(name, value);
        }
    }
}
void TouchPadManager::setPropToDevices(const QString &name, const QVector<bool> &values)
{
    auto inputDevices = m_inputBackend->getDevices();

    for (auto device : inputDevices)
    {
        if (device->hasProperty(name) && device->isTouchpad())
        {
            device->setProperty(name, values);
        }
    }
}
}  // namespace Kiran