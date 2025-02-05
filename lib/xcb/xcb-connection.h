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

#pragma once

#include <xcb/xproto.h>
#include <QObject>
#include <QSharedPointer>

typedef struct xcb_connection_t xcb_connection_t;

namespace Kiran
{

class XcbConnection : public QObject
{
    Q_OBJECT
public:
    XcbConnection(QObject *parent = nullptr);
    virtual ~XcbConnection();

    static QSharedPointer<XcbConnection> getDefault();

    xcb_connection_t *getConnection() const
    {
        return m_connection;
    }

    int getDefaultScreenNumber() const
    {
        return m_defaultScreenNumber;
    }

    xcb_screen_t *getDefaultScreen() const
    {
        return m_defaultScreen;
    }

    xcb_atom_t getAtom(const QString &name, bool onlyIfExists = true);

private:
    static QSharedPointer<XcbConnection> m_default;

    xcb_connection_t *m_connection;
    // 默认屏幕
    int m_defaultScreenNumber;
    xcb_screen_t *m_defaultScreen;
};

}  // namespace Kiran
