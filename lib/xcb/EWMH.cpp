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

#include "EWMH.h"
#include <glib.h>
#include <xcb/xcb.h>
#include <QCoreApplication>
#include <QRegExp>
#include "lib/base/base.h"
#include "xcb-connection.h"

namespace Kiran
{
#define NET_SUPPORTING_WM_CHECK "_NET_SUPPORTING_WM_CHECK"
#define NET_WM_NAME "_NET_WM_NAME"
#define MATE_WM_KEYBINDINGS "_MATE_WM_KEYBINDINGS"

EWMH::EWMH() : m_wmWindow(XCB_ATOM_NONE)
{
    m_xcbConnection = XcbConnection::getDefault();
}

EWMH::~EWMH()
{
}

QSharedPointer<EWMH> EWMH::m_instance = nullptr;
QSharedPointer<EWMH> EWMH::getDefault()
{
    if (m_instance.isNull())
    {
        m_instance = QSharedPointer<EWMH>::create();
        m_instance->init();
    }
    return m_instance;
}

QStringList EWMH::getWmKeybindings()
{
    auto keybindingsAtom = m_xcbConnection->getAtom(MATE_WM_KEYBINDINGS);
    auto keybindings = getWmProperty(keybindingsAtom);
    QStringList result;

    if (keybindings.length() > 0)
    {
        result = keybindings.split(QRegExp("\\s*,\\s*"));
    }
    else
    {
        auto wmAtom = m_xcbConnection->getAtom("_NET_WM_NAME");
        auto wmName = getWmProperty(wmAtom);
        if (wmName.length() > 0)
        {
            result.push_back(wmName);
        }
    }
    return result;
}

QString EWMH::getWmProperty(xcb_atom_t atom)
{
    auto utf8StringAtom = m_xcbConnection->getAtom("UTF8_STRING");

    RETURN_VAL_IF_TRUE(atom == XCB_ATOM_NONE, QString());
    RETURN_VAL_IF_TRUE(utf8StringAtom == XCB_ATOM_NONE, QString());
    RETURN_VAL_IF_TRUE(m_wmWindow == XCB_ATOM_NONE, QString());

    auto propertyReply = XCB_REPLY(xcb_get_property,
                                   m_xcbConnection->getConnection(),
                                   false,
                                   m_wmWindow,
                                   atom,
                                   utf8StringAtom,
                                   0,
                                   G_MAXINT32);
    if (!propertyReply ||
        propertyReply->type != utf8StringAtom ||
        propertyReply->format != 8)
    {
        KLOG_WARNING() << "Failed to get window manager property for atom" << atom;
        return QString();
    }

    auto propertyValue = QByteArray((const char*)xcb_get_property_value(propertyReply.get()),
                                    xcb_get_property_value_length(propertyReply.get()));

    return QString(propertyValue);
}

QString EWMH::getWmName()
{
    auto wmNameAtom = m_xcbConnection->getAtom(NET_WM_NAME);
    return getWmProperty(wmNameAtom);
}

void EWMH::init()
{
    auto rootWindowPropertyReply = XCB_REPLY(xcb_get_window_attributes,
                                             m_xcbConnection->getConnection(),
                                             m_xcbConnection->getDefaultScreen()->root);

    if (!rootWindowPropertyReply)
    {
        KLOG_WARNING() << "Failed to get window attributes for root window";
    }

    auto existedEvents = rootWindowPropertyReply ? rootWindowPropertyReply->your_event_mask : XCB_EVENT_MASK_NO_EVENT;
    const quint32 interestedEvents[] = {XCB_EVENT_MASK_PROPERTY_CHANGE | existedEvents};
    xcb_change_window_attributes(m_xcbConnection->getConnection(),
                                 m_xcbConnection->getDefaultScreen()->root,
                                 XCB_CW_EVENT_MASK,
                                 interestedEvents);

    QCoreApplication::instance()->installNativeEventFilter(this);

    updateWmWindow();
}

void EWMH::updateWmWindow()
{
    auto wmCheckAtom = m_xcbConnection->getAtom(NET_SUPPORTING_WM_CHECK);
    RETURN_IF_TRUE(wmCheckAtom == XCB_ATOM_NONE);

    auto wmXidReply = XCB_REPLY(xcb_get_property,
                                m_xcbConnection->getConnection(),
                                false,
                                m_xcbConnection->getDefaultScreen()->root,
                                wmCheckAtom,
                                XCB_ATOM_WINDOW,
                                0,
                                1);
    if (!wmXidReply || wmXidReply->type != XCB_ATOM_WINDOW || wmXidReply->format != 32)
    {
        KLOG_WARNING() << "Failed to get property for" << NET_SUPPORTING_WM_CHECK;
        return;
    }
    m_wmWindow = *((xcb_window_t*)xcb_get_property_value(wmXidReply.get()));

    const quint32 interestedEvents[] = {XCB_EVENT_MASK_STRUCTURE_NOTIFY | XCB_EVENT_MASK_PROPERTY_CHANGE};
    xcb_change_window_attributes(m_xcbConnection->getConnection(),
                                 m_wmWindow,
                                 XCB_CW_EVENT_MASK,
                                 interestedEvents);

    Q_EMIT wmWindowChange();
}

bool EWMH::nativeEventFilter(const QByteArray& eventType, void* message, long* result)
{
    if (eventType == "xcb_generic_event_t")
    {
        xcb_generic_event_t* ev = static_cast<xcb_generic_event_t*>(message);

        if ((ev->response_type == XCB_DESTROY_NOTIFY &&
             m_wmWindow != XCB_ATOM_NONE &&
             reinterpret_cast<xcb_destroy_notify_event_t*>(ev)->window == m_wmWindow) ||
            (ev->response_type == XCB_PROPERTY_NOTIFY &&
             reinterpret_cast<xcb_property_notify_event_t*>(ev)->window == m_xcbConnection->getDefaultScreen()->root &&
             reinterpret_cast<xcb_property_notify_event_t*>(ev)->atom == m_xcbConnection->getAtom(NET_SUPPORTING_WM_CHECK)) ||
            (ev->response_type == XCB_PROPERTY_NOTIFY &&
             m_wmWindow != XCB_ATOM_NONE &&
             reinterpret_cast<xcb_property_notify_event_t*>(ev)->window == m_wmWindow &&
             reinterpret_cast<xcb_property_notify_event_t*>(ev)->atom == m_xcbConnection->getAtom(NET_WM_NAME)))
        {
            updateWmWindow();
        }
    }
    return false;
}

}  // namespace Kiran