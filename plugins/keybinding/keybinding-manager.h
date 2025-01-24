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

#include <QDBusContext>
#include <QSharedPointer>

class KeybindingAdaptor;

namespace Kiran
{

class CustomShortcuts;
class SystemShortcuts;
class SystemShortcut;
class KeysComponent;

class KeybindingManager : public QObject,
                          protected QDBusContext
{
    Q_OBJECT

public:
    KeybindingManager();
    virtual ~KeybindingManager();

    static KeybindingManager *getInstance() { return m_instance; };
    static void globalInit();
    static void globalDeinit() { delete m_instance; };

public Q_SLOTS:
    // 添加自定义快捷键
    QString AddCustomShortcut(const QString &name, const QString &action, const QString &keyComb);
    // 删除自定义快捷键
    void DeleteCustomShortcut(const QString &uid);
    // 获取指定自定义快捷键
    QString GetCustomShortcut(const QString &uid);
    // 获取指定系统快捷键
    QString GetSystemShortcut(const QString &uid);
    // 获取所有自定义快捷键
    QString ListCustomShortcuts();
    // 获取所有快捷键，包括自定义和系统快捷键
    QString ListShortcuts();
    // 获取所有系统快捷键
    QString ListSystemShortcuts();
    // 修改系统快捷键
    void ModifyCustomShortcut(const QString &uid,
                              const QString &name,
                              const QString &action,
                              const QString &keyComb);
    // 修改自定义快捷键
    void ModifySystemShortcut(const QString &uid, const QString &keyComb);
    // 重置所有的系统快捷键
    void ResetShortcuts();

Q_SIGNALS:
    void Added(const QString &result);
    void Changed(const QString &result);
    void Deleted(const QString &result);

private:
    void init();

    // 判断快捷键是否已经存在
    bool hasSameKeycomb(const QString &uid, const QString &key_combination);

    void processSystemShortcutAdded(QSharedPointer<SystemShortcut> system_shortcut);
    void processSystemShortcutDeleted(QSharedPointer<SystemShortcut> system_shortcut);
    void processSystemShortcutChanged(QSharedPointer<SystemShortcut> system_shortcut);

private:
    static KeybindingManager *m_instance;
    KeybindingAdaptor *m_adaptor;
    QSharedPointer<CustomShortcuts> m_customShortcuts;
    QSharedPointer<SystemShortcuts> m_systemShortcuts;
    QVector<KeysComponent *> m_keysComponents;
};
}  // namespace Kiran