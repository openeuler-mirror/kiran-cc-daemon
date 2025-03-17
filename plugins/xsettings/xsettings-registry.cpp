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

#include "xsettings-registry.h"
#include <xcb/xcb.h>
#include <QTimer>
#include "lib/base/base.h"
#include "lib/xcb/xcb-connection.h"

namespace Kiran
{
#define XSETTINGS_PAD(n, m) ((n + m - 1) & (~(m - 1)))

XSettingsPropertyBase::XSettingsPropertyBase(const QString &name,
                                             XSettingsPropType type,
                                             uint32_t serial) : m_name(name),
                                                                m_type(type),
                                                                m_lastChangeSerial(serial)
{
}

QByteArray XSettingsPropertyBase::serialize()
{
    QByteArray data;
    data.push_back(char(m_type));
    data.push_back(char('\0'));

    // 类型不能随意修改
    uint16_t nameLen = m_name.length();
    uint16_t nameLenPad = XSETTINGS_PAD(nameLen, 4) - nameLen;
    data.append(QByteArray((const char *)&nameLen, 2));
    data.append(m_name.toUtf8());
    data.append(QByteArray(nameLenPad, '\0'));
    data.append(QByteArray((const char *)&m_lastChangeSerial, 4));
    return data;
}

XSettingsPropertyInt::XSettingsPropertyInt(const QString &name,
                                           int32_t value,
                                           uint32_t serial) : XSettingsPropertyBase(name, XSettingsPropType::XSETTINGS_PROP_TYPE_INT, serial),
                                                              m_value(value)
{
}

bool XSettingsPropertyInt::operator==(const XSettingsPropertyBase &rval) const
{
    if (rval.getType() != XSettingsPropType::XSETTINGS_PROP_TYPE_INT)
    {
        KLOG_WARNING(xsettings) << "Unsupported.";
        return false;
    }
    return operator==(dynamic_cast<const XSettingsPropertyInt &>(rval));
}

bool XSettingsPropertyInt::operator==(const XSettingsPropertyInt &rval) const
{
    return (getName() == rval.getName() && m_value == rval.m_value);
}

QByteArray XSettingsPropertyInt::serialize()
{
    QByteArray data;
    data = XSettingsPropertyBase::serialize();
    data.append(QByteArray((const char *)&m_value, 4));
    return data;
}

XSettingsPropertyString::XSettingsPropertyString(const QString &name,
                                                 const QString &value,
                                                 uint32_t serial) : XSettingsPropertyBase(name, XSettingsPropType::XSETTINGS_PROP_TYPE_STRING, serial),
                                                                    m_value(value)
{
}

bool XSettingsPropertyString::operator==(const XSettingsPropertyBase &rval) const
{
    if (rval.getType() != XSettingsPropType::XSETTINGS_PROP_TYPE_STRING)
    {
        KLOG_WARNING(xsettings) << "Unsupported.";
        return false;
    }
    return operator==(dynamic_cast<const XSettingsPropertyString &>(rval));
}

bool XSettingsPropertyString::operator==(const XSettingsPropertyString &rval) const
{
    return (getName() == rval.getName() && m_value == rval.m_value);
}

QByteArray XSettingsPropertyString::serialize()
{
    QByteArray data;
    data = XSettingsPropertyBase::serialize();
    uint32_t strLen = m_value.length();
    uint32_t strLenPad = XSETTINGS_PAD(strLen, 4) - strLen;
    data.append(QByteArray((const char *)&strLen, 4));
    data.append(m_value.toUtf8());
    data.append(QByteArray(strLenPad, '\0'));
    return data;
}

XSettingsPropertyColor::XSettingsPropertyColor(const QString &name,
                                               const XSettingsColor &value,
                                               uint32_t serial) : XSettingsPropertyBase(name, XSettingsPropType::XSETTINGS_PROP_TYPE_COLOR, serial),
                                                                  m_value(value)
{
}

bool XSettingsPropertyColor::operator==(const XSettingsPropertyBase &rval) const
{
    if (rval.getType() != XSettingsPropType::XSETTINGS_PROP_TYPE_COLOR)
    {
        KLOG_WARNING(xsettings) << "Unsupported.";
        return false;
    }
    return operator==(dynamic_cast<const XSettingsPropertyColor &>(rval));
}

bool XSettingsPropertyColor::operator==(const XSettingsPropertyColor &rval) const
{
    return (getName() == rval.getName() && m_value == rval.m_value);
}

QByteArray XSettingsPropertyColor::serialize()
{
    QByteArray data;
    data = XSettingsPropertyBase::serialize();
    data.append(QByteArray((const char *)&m_value.red, 2));
    data.append(QByteArray((const char *)&m_value.green, 2));
    data.append(QByteArray((const char *)&m_value.blue, 2));
    data.append(QByteArray((const char *)&m_value.alpha, 2));
    return data;
}

XSettingsRegistry::XSettingsRegistry(QObject *parent) : QObject(parent),
                                                        m_serial(0)
{
    m_xcbConnection = XcbConnection::getDefault();

    auto name = QString("_XSETTINGS_S%1").arg(m_xcbConnection->getDefaultScreenNumber());
    auto reply = XCB_REPLY(xcb_intern_atom, m_xcbConnection->getConnection(), false, name.size(), name.toUtf8().data());
    m_selectionAtom = reply ? reply->atom : XCB_ATOM_NONE;

    reply = XCB_REPLY(xcb_intern_atom, m_xcbConnection->getConnection(), false, strlen("_XSETTINGS_SETTINGS"), "_XSETTINGS_SETTINGS");
    m_xsettingsAtom = reply ? reply->atom : XCB_ATOM_NONE;

    reply = XCB_REPLY(xcb_intern_atom, m_xcbConnection->getConnection(), false, strlen("MANAGER"), "MANAGER");
    m_managerAtom = reply ? reply->atom : XCB_ATOM_NONE;

    m_timer = new QTimer(this);
}

XSettingsRegistry::~XSettingsRegistry()
{
}

bool XSettingsRegistry::init()
{
    // 检查是否有其他xsettings插件已经在运行

    auto xcbConnection = m_xcbConnection->getConnection();
    auto rootWindow = m_xcbConnection->getDefaultScreen()->root;

    auto reply = XCB_REPLY(xcb_get_selection_owner, xcbConnection, m_selectionAtom);
    if (reply && reply->owner != XCB_NONE)
    {
        KLOG_WARNING(xsettings) << "You can only run one xsettings manager at a time.";
        return false;
    }

    m_xsettingsWindow = xcb_generate_id(xcbConnection);
    xcb_create_window(xcbConnection,
                      XCB_COPY_FROM_PARENT,  // depth -- same as root
                      m_xsettingsWindow,     // window id
                      rootWindow,            // root window id
                      0, 0, 10, 10,
                      0,                     // border width
                      XCB_COPY_FROM_PARENT,  // window class
                      XCB_COPY_FROM_PARENT,  // visual
                      0,                     // value mask
                      nullptr);              // value list

    auto name = QString("_XSETTINGS_S%1").arg(m_xcbConnection->getDefaultScreenNumber());
    xcb_set_selection_owner(xcbConnection, m_xsettingsWindow, m_selectionAtom, XCB_TIME_CURRENT_TIME);
    // 判断设置Owner是否成功
    auto getSelectionReply = XCB_REPLY(xcb_get_selection_owner, xcbConnection, m_selectionAtom);
    if (!getSelectionReply)
    {
        KLOG_WARNING(xsettings) << "Failed to get selection owner for" << name;
        return false;
    }
    if (getSelectionReply->owner != m_xsettingsWindow)
    {
        KLOG_WARNING(xsettings) << "Failed to set selection owner" << m_xsettingsWindow
                                << "for" << name << "which not equal to" << getSelectionReply->owner;
        return false;
    }

    KLOG_INFO(xsettings) << "Success to set owner selection" << name << "to" << m_xsettingsWindow
                         << ", parent window is" << rootWindow;

    // 其他客户端通过监听"MANAGER"事件来感知Xsettings Owner是否创建
    xcb_client_message_event_t event;
    event.response_type = XCB_CLIENT_MESSAGE;
    event.window = rootWindow;
    event.format = 32;
    event.type = m_managerAtom;
    event.data.data32[0] = XCB_TIME_CURRENT_TIME;
    event.data.data32[1] = m_selectionAtom;
    event.data.data32[2] = m_xsettingsWindow;
    event.data.data32[3] = 0;
    event.data.data32[4] = 0;
    xcb_send_event(xcbConnection, false, rootWindow, XCB_EVENT_MASK_STRUCTURE_NOTIFY, (char *)&event);

    connect(m_timer, &QTimer::timeout, this, &XSettingsRegistry::notify);

    return true;
}

bool XSettingsRegistry::update(const QString &name, int32_t value)
{
    auto var = QSharedPointer<XSettingsPropertyInt>::create(name, value, m_serial);
    return update(var);
}

bool XSettingsRegistry::update(const QString &name, const QString &value)
{
    auto var = QSharedPointer<XSettingsPropertyString>::create(name, value, m_serial);
    return update(var);
}

bool XSettingsRegistry::update(const QString &name, const XSettingsColor &value)
{
    auto var = QSharedPointer<XSettingsPropertyColor>::create(name, value, m_serial);
    return update(var);
}

bool XSettingsRegistry::update(QSharedPointer<XSettingsPropertyBase> var)
{
    RETURN_VAL_IF_TRUE(var == nullptr, true);
    auto oldVar = getProperty(var->getName());
    if (oldVar != nullptr && *oldVar == *var)
    {
        return true;
    }

    m_changedProperties.push_back(var->getName());
    m_properties.remove(var->getName());
    m_properties.insert(var->getName(), var);

    // 空闲时修改，因为update可能会被连续调用多次。
    m_timer->start(100);
    return true;
}

void XSettingsRegistry::notify()
{
    QByteArray data;

    m_timer->stop();

    // 注意：填充的相关变量类型不能随意修改
    // 填充head：byte-order + pad + SERIAL + N_SETTINGS
    int32_t nsettings = m_properties.size();
    data.push_back(byteOrder());
    data.append(QByteArray("\0\0\0", 3));
    data.append(QByteArray((const char *)&m_serial, 4));
    data.append(QByteArray((const char *)&nsettings, 4));
    ++m_serial;

    // 填充body
    for (const auto &property : m_properties)
    {
        data.append(property->serialize());
    }

    KLOG_INFO(xsettings) << "Changed _XSETTINGS_SETTINGS properties to" << data;

    xcb_change_property(m_xcbConnection->getConnection(),
                        XCB_PROP_MODE_REPLACE,
                        m_xsettingsWindow,
                        m_xsettingsAtom,
                        m_xsettingsAtom,
                        8,
                        data.length(),
                        data.data());

    /* 这里必须要刷新，否则数据可能在缓存中导致未立即生效，引发一些问题，例如设置QT光标不生效：
    1. 通过控制中心前端设置光标主题
    2. 控制中心后端收到设置光标主题dbus请求，修改gsettings的gtk-cursor-theme-name
        2.1 启动定制器(100ms延时），定时器触发后控制中心后端执行当前函数，更新_XSETTINGS_SETTINGS
        2.2 kiran QPA主题插件启动定时器（500ms延时），定时器触发后告知QT刷新光标主题，QT从_XSETTINGS_SETTINGS中获取数据进行刷新
    如果2.1步中更新_XSETTINGS_SETTINGS到xserver没有立即生效（在缓存中），就会导致QT获取到的是旧的数据，导致光标主题设置失败，
    就算后续数据更新了，QT也不会再执行刷新动作。*/
    xcb_flush(m_xcbConnection->getConnection());

    auto changedProperties = std::move(m_changedProperties);
    Q_EMIT propertiesChanged(changedProperties);
}

char XSettingsRegistry::byteOrder()
{
    // sonarqube block off
    // 在指定机器上，下面的三元运算符永远为true，所以sonarqube会报code smell，这是正常的
    uint32_t myint = 0x01020304;
    return (*reinterpret_cast<char *>(&myint) == 1) ? XCB_IMAGE_ORDER_MSB_FIRST : XCB_IMAGE_ORDER_LSB_FIRST;
    // sonarqube block on
}

}  // namespace Kiran
