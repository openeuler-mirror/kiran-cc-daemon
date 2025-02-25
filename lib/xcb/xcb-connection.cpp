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

#include "xcb-connection.h"
#include <xcb/xcb.h>
#include <QX11Info>
#include "lib/base/base.h"

namespace Kiran
{
XcbConnection::XcbConnection(QObject *parent) : QObject(parent)
{
    m_connection = QX11Info::connection();
    m_defaultScreenNumber = QX11Info::appScreen();

    auto setup = xcb_get_setup(m_connection);
    auto iter = xcb_setup_roots_iterator(setup);

    for (int i = 0; i < m_defaultScreenNumber; ++i)
    {
        xcb_screen_next(&iter);
    }
    m_defaultScreen = iter.data;
}

XcbConnection ::~XcbConnection()
{
}

QSharedPointer<XcbConnection> XcbConnection::m_default = nullptr;
QSharedPointer<XcbConnection> XcbConnection::getDefault()
{
    if (m_default.isNull())
    {
        m_default = QSharedPointer<XcbConnection>::create();
    }
    return m_default;
}

xcb_atom_t XcbConnection::getAtom(const QString &name, bool onlyIfExists)
{
    auto atomReply = XCB_REPLY(xcb_intern_atom,
                               m_connection,
                               onlyIfExists,
                               name.length(),
                               name.toLatin1().data());

    if (!atomReply)
    {
        KLOG_WARNING() << "Failed to get atom for" << name;
        return XCB_ATOM_NONE;
    }
    return atomReply->atom;
}
}  // namespace Kiran
