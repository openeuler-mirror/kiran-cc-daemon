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

#include <QHash>
#include "keys-component.h"

class QGSettings;

namespace Kiran
{
class KeysSystem : public KeysComponent
{
    Q_OBJECT

public:
    KeysSystem();
    virtual ~KeysSystem(){};

    virtual void init() override;

private:
    void registerConfigurableShortcuts(bool forceUpdate);
    void registerConfigurableShortCut(const QString &schemaKey, bool forceUpdate);
    void processSettingsChanged(const QString &schemaKey);
    void processKGlobalShortcutChanged(const QString &actionUnique);
    QString schemaKeyByAction(const QString &actionUnique) const;
    QString actionBySchemaKey(const QString &schemaKey) const;
    QString displayNameByAction(const QString &actionUnique) const;

    void launchHelp();
    void lockScreen();
    void showdesktop();
    void popupStartMenu();
    void logout();
    void shutdown();
    void openHome();
    void openCalculator();
    void openSearch();
    void openControlCenter();

private:
    virtual void triggerShortCut(const QString &name) override;
    QHash<QString, QString> m_actionSchemaKeyMap;
    QHash<QString, QString> m_schemaActionMap;
    bool m_syncingFromSettings;
    bool m_syncingFromKGlobalAccel;
};

}  // namespace Kiran
