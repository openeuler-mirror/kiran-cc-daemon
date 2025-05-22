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
#include "keybinding-i.h"
#include "lib/base/base.h"

namespace Kiran
{
KeysComponent::KeysComponent(const QString &componentName,
                             const QString &displayName) : m_componentInterface(nullptr)
{
    m_settings = new QGSettings(KEYS_SCHEMA_ID, "", this);
    m_actionCollection = new KActionCollection(this);
    m_actionCollection->setComponentName(componentName);
    m_actionCollection->setComponentDisplayName(displayName);

    KGlobalAccelInterface globalAccelInterface(QStringLiteral("org.kde.kglobalaccel"),
                                               QStringLiteral("/kglobalaccel"),
                                               QDBusConnection::sessionBus());

    // 触发kglobalaccel创建组件并加载配置
    QStringList actionId = KeybindingUtils::buildActionId(componentName, displayName, QString(), QString());
    globalAccelInterface.doRegister(actionId);
    globalAccelInterface.unRegister(actionId);

    auto componentReply = globalAccelInterface.getComponent(componentName);
    componentReply.waitForFinished();
    if (componentReply.isError())
    {
        KLOG_WARNING(keybinding) << "Failed to get objectPath for component" << componentName << "which error is" << componentReply.error();
    }
    else
    {
        auto componentPath = componentReply.value();
        m_componentInterface = new KGlobalAccelComponentInterface(globalAccelInterface.service(),
                                                                  componentPath.path(),
                                                                  globalAccelInterface.connection(),
                                                                  this);

        connect(m_componentInterface, &KGlobalAccelComponentInterface::globalShortcutPressed, this, &KeysComponent::processShortcutPressed);
    }
}

bool KeysComponent::registerShortCut(const QKeySequence &key,
                                     const QString &name,
                                     const QString &displayName)
{
    QAction *globalAction = m_actionCollection->addAction(name);
    globalAction->setText(displayName);
    return KGlobalAccel::self()->setGlobalShortcut(globalAction, QList<QKeySequence>() << key);
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

    this->triggerShortCut(actionUnique);
}

}  // namespace Kiran
