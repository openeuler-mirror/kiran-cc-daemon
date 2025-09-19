/**
 * Copyright (c) 2023 ~ 2024 KylinSec Co., Ltd.
 * kiran-cc-daemon is licensed under Mulan PSL v2.
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
#include <QRegularExpressionMatch>
#include "lib/base/base.h"

namespace Kiran
{

// 修正下面的映射表，将右边的符号改成QT能识别的字符串

static const QMap<QString, QString> s_gtk2QtSymbols = {
    {"Escape", "Esc"},
    {"Insert", "Ins"},
    {"KP_Delete", "Del"},
    {"Page_Down", "PgDown"},
    {"Page_Up", "PgUp"},
    {"ampersand", "&"},
    {"apostrophe", "'"},
    {"asciicircum", "^"},
    {"asciitilde", "~"},
    {"asterisk", "*"},
    {"at", "@"},
    {"backslash", "\\"},
    {"bar", "|"},
    {"braceleft", "{"},
    {"braceright", "}"},
    {"bracketleft", "["},
    {"bracketright", "]"},
    {"colon", ":"},
    {"comma", ","},
    {"dollar", "$"},
    {"equal", "="},
    {"exclam", "!"},
    {"grave", "`"},
    {"greater", ">"},
    {"less", "<"},
    {"minus", "-"},
    {"numbersign", "#"},
    {"parenleft", "("},
    {"parenright", ")"},
    {"percent", "%"},
    {"period", "."},
    {"plus", "+"},
    {"question", "?"},
    {"quotedbl", "\""},
    {"quoteleft", "`"},
    {"semicolon", ";"},
    {"slash", "/"},
    {"underscore", "_"}};

static const QMap<QString, QString> s_qt2GtkSymbols = {
    {"!", "exclam"},
    {"\"", "quotedbl"},
    {"#", "numbersign"},
    {"$", "dollar"},
    {"%", "percent"},
    {"&", "ampersand"},
    {"'", "apostrophe"},
    {"(", "parenleft"},
    {")", "parenright"},
    {"*", "asterisk"},
    {"+", "plus"},
    {",", "comma"},
    {"-", "minus"},
    {".", "period"},
    {"/", "slash"},
    {":", "colon"},
    {";", "semicolon"},
    {"<", "less"},
    {"=", "equal"},
    {">", "greater"},
    {"?", "question"},
    {"@", "at"},
    {"Del", "KP_Delete"},
    {"Esc", "Escape"},
    {"Ins", "Insert"},
    {"PgDown", "Page_Down"},
    {"PgUp", "Page_Up"},
    {"[", "bracketleft"},
    {"\\", "backslash"},
    {"]", "bracketright"},
    {"^", "asciicircum"},
    {"_", "underscore"},
    {"`", "grave"},
    {"{", "braceleft"},
    {"|", "bar"},
    {"}", "braceright"},
    {"~", "asciitilde"}};

KeybindingUtils::KeybindingUtils()
{
}

bool KeybindingUtils::isValidKeySequence(const QString &keyComb)
{
    if (keyComb.isEmpty() || keyComb.compare("disabled", Qt::CaseInsensitive) == 0)
    {
        return true;
    }

    if (QKeySequence::fromString(keyComb).toString().isEmpty())
    {
        KLOG_WARNING(keybinding) << "The format of the key combination" << keyComb << "is invalid.";
        return false;
    }
    return true;
}

bool KeybindingUtils::isValidGtkkeyComb(const QString &keyCombGtk)
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
    QRegularExpressionMatch matchModifier;
    keyCombGtk.indexOf(QRegularExpression("^(<[a-zA-Z]+>)+"), 0, &matchModifier);
    auto keyCombModifier = matchModifier.captured();
    auto keyCombSymbol = keyCombGtk.right(keyCombGtk.length() - keyCombModifier.length());

    keyCombModifier.replace(QRegExp("<super>", Qt::CaseInsensitive), "<Meta>");
    keyCombModifier.replace(QRegExp("<control>", Qt::CaseInsensitive), "<Ctrl>");
    keyCombModifier.replace("<", "");
    keyCombModifier.replace(">", "+");

    if (s_gtk2QtSymbols.contains(keyCombSymbol))
    {
        keyCombSymbol = s_gtk2QtSymbols[keyCombSymbol];
    }
    auto keyCombQt = keyCombModifier + keyCombSymbol;
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
    auto keyCombSymbol = keyCombQtTokens.last();
    if (s_qt2GtkSymbols.contains(keyCombSymbol))
    {
        keyCombSymbol = s_qt2GtkSymbols[keyCombSymbol];
    }
    keyCombGtk.append(keyCombSymbol);
    keyCombGtk.replace(QRegExp("<meta>", Qt::CaseInsensitive), "<Super>");
    keyCombGtk.replace(QRegExp("<ctrl>", Qt::CaseInsensitive), "<Control>");
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
