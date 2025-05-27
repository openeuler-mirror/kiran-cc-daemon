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

#include <QObject>

class KActionCollection;
class QGSettings;
class KGlobalAccelComponentInterface;

namespace Kiran
{
class KeysComponent : public QObject
{
    Q_OBJECT

public:
    explicit KeysComponent(const QString &componentName, const QString &displayName);

public:
    virtual void init() = 0;
    virtual void triggerShortCut(const QString &name) = 0;

protected:
    /*
    @param key 监听的按键
    @param name 按键名称
    @param displayName 按键显示名称
    @param isPressed 为true则监听按下信号，为false则监听弹起信号
    @param forceUpdate 为true时，无论快捷键是否已经存在，都强制更新当前快捷键
    */
    bool registerShortCut(const QKeySequence &key,
                          const QString &name,
                          const QString &displayName,
                          bool isPressed = true,
                          bool forceUpdate = false);

    bool registerShortCut(const QList<QKeySequence> &keys,
                          const QString &name,
                          const QString &displayName,
                          bool isPressed = true,
                          bool forceUpdate = false);

private:
    void processShortcutPressed(const QString &componentUnique,
                                const QString &actionUnique,
                                qlonglong timestamp);

    void processShortcutReleased(const QString &componentUnique,
                                 const QString &actionUnique,
                                 qlonglong timestamp);

protected:
    QGSettings *m_settings;

private:
    KActionCollection *m_actionCollection;
    KGlobalAccelComponentInterface *m_componentInterface;
    bool m_pressedTriggered;
};
}  // namespace Kiran