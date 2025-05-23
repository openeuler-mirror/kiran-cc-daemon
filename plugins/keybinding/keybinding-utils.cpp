/**
 * Copyright (c) 2023 ~ 2024 KylinSec Co., Ltd.
 * ks-ssr is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     tangjie02 <tangjie02@kylinsec.com.cn>
 */

#include "keybinding-utils.h"
#include <KGlobalAccel>
#include <QKeySequence>
#include "lib/base/base.h"

namespace Kiran
{
KeybindingUtils::KeybindingUtils()
{
}

bool KeybindingUtils::isValidKeySequence(const QString &keyCombGtk)
{
    if (keyCombGtk.isEmpty() || keyCombGtk.compare("disabled", Qt::CaseInsensitive) == 0)
    {
        return true;
    }

    auto keyCombQt = KeybindingUtils::keyCombGtk2Qt(keyCombGtk);
    return (QKeySequence::fromString(keyCombQt).toString().isEmpty() == false);
}

QString KeybindingUtils::keyCombGtk2Qt(const QString &keyCombGtk)
{
    auto keyCombQt = keyCombGtk;
    keyCombQt.replace(QRegExp("<super>", Qt::CaseInsensitive), "<Meta>");
    keyCombQt.replace(QRegExp("<control>", Qt::CaseInsensitive), "<Ctrl>");
    keyCombQt.replace("<", "");
    keyCombQt.replace(">", "+");
    return keyCombQt;
}

QString KeybindingUtils::keyCombQt2Gtk(const QString &keyCombQt)
{
    RETURN_VAL_IF_TRUE(keyCombQt.isEmpty(), QString());

    QString keyCombGtk;
    auto keyCombQtTokens = keyCombQt.split("+");

    for (int i = 0; i + 1 < keyCombQtTokens.size(); i++)
    {
        keyCombGtk.append(QString("<%1>").arg(keyCombQtTokens[i]));
    }
    keyCombGtk.append(keyCombQtTokens.last());
    keyCombGtk.replace(QRegExp("<meta>", Qt::CaseInsensitive), "<Super>");
    // keyCombGtk.replace(QRegExp("<ctrl>", Qt::CaseInsensitive), "<Control>");
    return keyCombGtk;
}

QStringList KeybindingUtils::buildActionId(const QString &componentUnique,
                                           const QString &componentFriendly,
                                           const QString &actionUnique,
                                           const QString &actionFriendly)
{
    QStringList actionId{"", "", "", ""};
    actionId[KGlobalAccel::ComponentUnique] = componentUnique;
    actionId[KGlobalAccel::ComponentFriendly] = componentFriendly;
    actionId[KGlobalAccel::ActionUnique] = actionUnique;
    actionId[KGlobalAccel::ActionFriendly] = actionFriendly;
    return actionId;
}
}  // namespace Kiran
