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
#include <KActionCollection>
#include <KGlobalAccel>
#include <QAction>
#include <QGSettings>
#include "keybinding-i.h"

namespace Kiran
{

KeysComponent::KeysComponent(const QString &componentName, const QString &displayName)
{
    m_settings = new QGSettings(KEYS_SCHEMA_ID, "", this);
    m_actionCollection = new KActionCollection(this);
    m_actionCollection->setComponentName(componentName);
    m_actionCollection->setComponentDisplayName(displayName);
}

bool KeysComponent::registerShortCut(const QKeySequence &key, const QString &name, const QString &displayName)
{
    QAction *globalAction = m_actionCollection->addAction(name);
    globalAction->setText(displayName);
    connect(globalAction, &QAction::triggered, this, [this, name]
            { this->triggerShortCut(name); });

    return KGlobalAccel::self()->setGlobalShortcut(globalAction, QList<QKeySequence>() << key);
}

}  // namespace Kiran
