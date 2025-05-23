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

#pragma once

#include <xcb/xproto.h>
#include <QAbstractNativeEventFilter>
#include <QSharedPointer>
#include <QStringList>

namespace Kiran
{
class XcbConnection;

// Extended Window Manager Hints
class EWMH : public QObject,
             public QAbstractNativeEventFilter
{
    Q_OBJECT

public:
    EWMH();
    virtual ~EWMH();

    static QSharedPointer<EWMH> getDefault();

    // 获得窗口管理器keybingding名字列表
    QStringList getWmKeybindings();

    // 获取窗口管理器的窗口属性
    QString getWmProperty(xcb_atom_t atom);

    // 获取窗口管理器名字
    QString getWmName();

Q_SIGNALS:
    void wmWindowChanged();

private:
    void init();
    // 更新窗口管理器在根窗口中设置的_NET_SUPPORTING_WM_CHECK属性，该属性值记录了窗口管理器的一个子窗口
    void updateWmWindow();
    bool nativeEventFilter(const QByteArray &eventType, void *message, long *result) override;

private:
    static QSharedPointer<EWMH> m_instance;
    QSharedPointer<XcbConnection> m_xcbConnection;
    xcb_window_t m_wmWindow;
};
}  // namespace Kiran