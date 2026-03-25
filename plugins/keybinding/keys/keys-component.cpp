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

#include "keys-component.h"
#include <kglobalaccel_component_interface.h>
#include <kglobalaccel_interface.h>
#include <KActionCollection>
#include <KGlobalAccel>
#include <QAction>
#include <QGSettings>
#include <QGuiApplication>
#include <QTimer>
#include "../keybinding-utils.h"
#include "../kglobalaccel-dbus-types.h"
#include "keybinding-i.h"
#include "lib/base/base.h"

namespace Kiran
{
KeysComponent::KeysComponent(const QString &componentName,
                             const QString &displayName) : m_componentInterface(nullptr),
                                                           m_globalAccelInterface(nullptr),
                                                           m_pressedTriggered(false)
{
    m_settings = new QGSettings(KEYS_SCHEMA_ID, "", this);
    m_actionCollection = new KActionCollection(this);
    m_actionCollection->setComponentName(componentName);
    m_actionCollection->setComponentDisplayName(displayName);

    qDBusRegisterMetaType<QList<int>>();
    qDBusRegisterMetaType<QKeySequence>();
    qDBusRegisterMetaType<QList<QKeySequence>>();
    qDBusRegisterMetaType<QList<QStringList>>();
    qDBusRegisterMetaType<KGlobalShortcutInfo>();
    qDBusRegisterMetaType<QList<KGlobalShortcutInfo>>();

    m_globalAccelInterface = new KGlobalAccelInterface(QStringLiteral("org.kde.kglobalaccel"),
                                                       QStringLiteral("/kglobalaccel"),
                                                       QDBusConnection::sessionBus(),
                                                       this);

    // 触发kglobalaccel创建组件并加载配置
    QStringList actionId = KeybindingUtils::buildActionId(componentName, displayName, QString(), QString());
    m_globalAccelInterface->doRegister(actionId);
    m_globalAccelInterface->unRegister(actionId);

    auto componentReply = m_globalAccelInterface->getComponent(componentName);
    componentReply.waitForFinished();
    if (componentReply.isError())
    {
        KLOG_WARNING(keybinding) << "Failed to get objectPath for component" << componentName << "which error is" << componentReply.error();
    }
    else
    {
        auto componentPath = componentReply.value();
        m_componentInterface = new KGlobalAccelComponentInterface(m_globalAccelInterface->service(),
                                                                  componentPath.path(),
                                                                  m_globalAccelInterface->connection(),
                                                                  this);

        connect(m_componentInterface, &KGlobalAccelComponentInterface::globalShortcutPressed, this, &KeysComponent::processShortcutPressed);
        connect(m_componentInterface, &KGlobalAccelComponentInterface::globalShortcutReleased, this, &KeysComponent::processShortcutReleased);
    }

    connect(m_globalAccelInterface, &KGlobalAccelInterface::yourShortcutsChanged, this, &KeysComponent::processYourShortcutsChanged);
}

bool KeysComponent::registerShortCut(const QKeySequence &key,
                                     const QString &name,
                                     const QString &displayName,
                                     bool isPressed,
                                     bool forceUpdate)
{
    return registerShortCut(QList<QKeySequence>() << key, name, displayName, isPressed, forceUpdate);
}

bool KeysComponent::registerShortCut(const QList<QKeySequence> &keys,
                                     const QString &name,
                                     const QString &displayName,
                                     bool isPressed,
                                     bool forceUpdate)
{
    QAction *globalAction = m_actionCollection->addAction(name);
    globalAction->setText(displayName);
    globalAction->setData(isPressed);

    if (forceUpdate)
    {
        KGlobalAccel::self()->setShortcut(globalAction, keys, KGlobalAccel::NoAutoloading);
    }

    return KGlobalAccel::self()->setGlobalShortcut(globalAction, keys);
}

QAction *KeysComponent::getAction(const QString &name) const
{
    return m_actionCollection->action(name);
}

QString KeysComponent::getComponentName() const
{
    return m_actionCollection->componentName();
}

void KeysComponent::processShortcutPressed(const QString &componentUnique,
                                           const QString &actionUnique,
                                           qlonglong timestamp)
{
    if (m_actionCollection->componentName() != componentUnique)
    {
        // 正常情况不会走到这里
        KLOG_ERROR(keybinding) << "Both components" << m_actionCollection->componentName() << "and" << componentUnique << "are not same";
        return;
    }

    KLOG_INFO(keybinding) << "Receive shortcut pressed signal "
                          << "which component name is" << componentUnique
                          << "and action name is" << actionUnique
                          << "at time" << timestamp;

    auto action = m_actionCollection->action(actionUnique);
    if (action && action->data().toBool())
    {
        m_pressedTriggered = true;
        this->triggerShortCut(actionUnique);
    }
}

void KeysComponent::processShortcutReleased(const QString &componentUnique,
                                            const QString &actionUnique,
                                            qlonglong timestamp)
{
    if (m_actionCollection->componentName() != componentUnique)
    {
        // 正常情况不会走到这里
        KLOG_ERROR(keybinding) << "Both components" << m_actionCollection->componentName() << "and" << componentUnique << "are not same";
        return;
    }

    KLOG_INFO(keybinding) << "Receive shortcut released signal "
                          << "which component name is" << componentUnique
                          << "and action name is" << actionUnique
                          << "at time" << timestamp;

    auto action = m_actionCollection->action(actionUnique);
    if (action && !action->data().toBool())
    {
        m_pressedTriggered = false;
        /* 考虑到如下场景：
              同时存在<super>（弹出开始菜单窗口）和<super>D（显示桌面）两个快捷键，第一个快捷键是按键弹起时触发，第二个是按下时触发。
              当<super>被按下时，会收到快捷键1（<super>）被按下信号；
              当D被按下时，kglobalaccel首先会发送快捷键1释放信号，然后发送快捷键2按下信号；
              当D被释放时，kglobalaccel会发送快捷键2释放信号；
              （<super>释放时不会再触发快捷键1释放信号）
           在这个场景中，如果不进行延时处理，两个快捷键的命令都会执行，与预期不符。所以需要针对释放信号进行延时处理，如果延时的这段时间
           收到了按下信号，说明存在两个快捷键都被命中了，这时不应该再触发释放信号对应的快捷键命令。*/
        QTimer::singleShot(50, [this, actionUnique]()
                           {
                               if (!this->m_pressedTriggered)
                               {
                                   this->triggerShortCut(actionUnique);
                                   // showdesktop功能在定时器中不会立即触发（可能是在xcb缓存队列中），因此这里进行强制同步处理
                                   QGuiApplication::sync();
                               } });
    }
}

void KeysComponent::processYourShortcutsChanged(const QStringList &actionId, const QList<QKeySequence> &newKeys)
{
    Q_UNUSED(newKeys);

    if (actionId.size() <= KGlobalAccel::ActionUnique || actionId.size() <= KGlobalAccel::ComponentUnique)
    {
        KLOG_WARNING(keybinding) << "Invalid action id from yourShortcutsChanged:" << actionId;
        return;
    }

    auto componentUnique = actionId.at(KGlobalAccel::ComponentUnique);
    auto actionUnique = actionId.at(KGlobalAccel::ActionUnique);
    if (componentUnique != m_actionCollection->componentName() || actionUnique.isEmpty())
    {
        return;
    }

    Q_EMIT shortcutChanged(actionUnique);
}

}  // namespace Kiran
