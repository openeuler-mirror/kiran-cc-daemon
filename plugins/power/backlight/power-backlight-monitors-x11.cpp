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

#include "power-backlight-monitors-x11.h"
#include <xcb/randr.h>
#include <xcb/xcb.h>
#include <QSocketNotifier>
#include <QX11Info>
#include "lib/xcb/xcb-connection.h"
#include "power-backlight-monitor-x11-atom.h"
#include "power-backlight-monitor-x11-gamma.h"

namespace Kiran
{
PowerBacklightMonitorsX11::PowerBacklightMonitorsX11() : m_xcbSocketNotifier(nullptr),
                                                         m_eventBase(0),
                                                         m_errorBase(0),
                                                         m_extensionSupported(false),
                                                         m_backlightAtom(XCB_ATOM_NONE)
{
    m_xcbConnection = new XcbConnection(this);
}

PowerBacklightMonitorsX11::~PowerBacklightMonitorsX11()
{
}

void PowerBacklightMonitorsX11::init()
{
    RETURN_IF_FALSE(initXrandr());

    m_backlightAtom = getBacklightAtom();
    loadResource();

    auto xcbConnection = m_xcbConnection->getConnection();
    auto defaultScreen = m_xcbConnection->getDefaultScreen();

    xcb_randr_select_input(xcbConnection, defaultScreen->root, XCB_RANDR_NOTIFY_MASK_SCREEN_CHANGE | XCB_RANDR_NOTIFY_MASK_OUTPUT_PROPERTY);
    m_extensionSupported = true;

    auto fd = xcb_get_file_descriptor(xcbConnection);
    m_xcbSocketNotifier = new QSocketNotifier(fd, QSocketNotifier::Read, this);
    connect(m_xcbSocketNotifier, &QSocketNotifier::activated, this, &PowerBacklightMonitorsX11::processXcbSocketActivated);
}

bool PowerBacklightMonitorsX11::initXrandr()
{
    auto xcbConnection = m_xcbConnection->getConnection();

    auto reply = xcb_get_extension_data(xcbConnection, &xcb_randr_id);
    if (!reply || !reply->present)
    {
        KLOG_WARNING(power) << "RANDR extension is not present";
        return false;
    }

    m_eventBase = reply->first_event;
    m_errorBase = reply->first_error;

    auto xrandrQuery = XCB_REPLY(xcb_randr_query_version, xcbConnection,
                                 XCB_RANDR_MAJOR_VERSION,
                                 XCB_RANDR_MINOR_VERSION);
    if (!xrandrQuery || (xrandrQuery->major_version < 1 ||
                         (xrandrQuery->major_version == 1 && xrandrQuery->minor_version < 3)))
    {
        KLOG_WARNING(power) << "RANDR extension is too old (must be at least 1.3). current version:"
                            << xrandrQuery->major_version << ":" << xrandrQuery->minor_version;
        return false;
    }

    return true;
}

xcb_atom_t PowerBacklightMonitorsX11::getBacklightAtom()
{
    auto xcbConnection = m_xcbConnection->getConnection();

    // 此属性适用于笔记本电脑和带背光控制器的显示器
    auto reply = XCB_REPLY(xcb_intern_atom, xcbConnection, true, strlen("Backlight"), "Backlight");
    if (!reply || reply->atom == XCB_ATOM_NONE)
    {
        // 兼容老的属性
        reply = XCB_REPLY(xcb_intern_atom, xcbConnection, true, strlen("BACKLIGHT"), "BACKLIGHT");
        if (!reply || reply->atom == XCB_ATOM_NONE)
        {
            KLOG_INFO(power) << "No outputs have backlight property";
            return XCB_ATOM_NONE;
        }
    }
    return reply->atom;
}

void PowerBacklightMonitorsX11::loadResource()
{
    auto xcbConnection = m_xcbConnection->getConnection();
    auto defaultScreen = m_xcbConnection->getDefaultScreen();

    m_backlightMonitors.clear();

    auto reply = XCB_REPLY(xcb_randr_get_screen_resources_current, xcbConnection, defaultScreen->root);
    RETURN_IF_TRUE(reply->num_outputs == 0);

    auto outputs = xcb_randr_get_screen_resources_current_outputs(reply.get());

    for (int i = 0; i < reply->num_outputs; ++i)
    {
        QSharedPointer<PowerBacklightAbsolute> monitor;
        auto outputInfo = XCB_REPLY_UNCHECKED(xcb_randr_get_output_info, xcbConnection, outputs[i], reply->config_timestamp);
        if (!outputInfo)
        {
            KLOG_WARNING(power) << "Not found output info for" << uint(outputs[i]);
            continue;
        }

        if (!outputInfo->crtc)
        {
            KLOG_DEBUG(power) << "Not found crtc for output" << uint(outputs[i]) << ", ignore it.";
            continue;
        }

        if (m_backlightAtom != XCB_ATOM_NONE)
        {
            monitor = QSharedPointer<PowerBacklightMonitorX11Atom>::create(m_backlightAtom, outputs[i]);
        }
        else
        {
            monitor = QSharedPointer<PowerBacklightMonitorX11Gamma>::create(outputs[i], outputInfo->crtc);
        }
        m_backlightMonitors.push_back(monitor);
    }
}

void PowerBacklightMonitorsX11::handleXcbEvent()
{
    while (xcb_generic_event_t *event = xcb_poll_for_event(m_xcbConnection->getConnection()))
    {
        if (!(event->response_type & ~0x80))
        {
            auto errorEvent = reinterpret_cast<xcb_generic_error_t *>(event);
            KLOG_WARNING() << "QXcbConnection: XCB error. error code:" << errorEvent->error_code;
            continue;
        }

        auto responseType = (event->response_type & ~0x80);

        switch (responseType - m_eventBase)
        {
        case XCB_RANDR_SCREEN_CHANGE_NOTIFY:
        {
            loadResource();
            Q_EMIT monitorChanged();
            break;
        }
        case XCB_RANDR_NOTIFY:
        {
            auto randrEvent = reinterpret_cast<xcb_randr_notify_event_t *>(event);
            if (randrEvent->subCode == XCB_RANDR_NOTIFY_OUTPUT_PROPERTY)
            {
                Q_EMIT brightnessChanged();
            }
            break;
        }
        default:
            break;
        }

        ::free(event);
    }
}

#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
void PowerBacklightMonitorsX11::processXcbSocketActivated(QSocketDescriptor socket,
                                                          QSocketNotifier::Type activationEvent)
{
    if (activationEvent != QSocketNotifier::Read)
    {
        return;
    }

    handleXcbEvent();
}
#else
void PowerBacklightMonitorsX11::processXcbSocketActivated(int socket)
{
    handleXcbEvent();
}
#endif

}  // namespace Kiran
