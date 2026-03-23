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

#include "keys-system.h"
#include <KDesktopFile>
#include <KGlobalAccel>
#include <KIO/DesktopExecParser>
#include <KService>
#include <KWindowSystem>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCall>
#include <QDir>
#include <QGSettings>
#include <QKeySequence>
#include <QProcess>
#include <QVariant>
#include <XdgDefaultApps>
#include <XdgDesktopFile>
#include "keybinding-i.h"
#include "lib/base/base.h"
#include "lib/base/def.h"

namespace Kiran
{
#define ACTION_NAME_HELP "help"
#define ACTION_NAME_EMAIL "email"
#define ACTION_NAME_SCREENSAVER "screensaver"
#define ACTION_NAME_SHOW_DESKTOP "showDesktop"
#define ACTION_NAME_START_MENU "startMenu"
#define ACTION_NAME_LOGOUT "logout"
#define ACTION_NAME_POWER "power"
#define ACTION_NAME_HOME "home"
#define ACTION_NAME_CALCULATOR "calculator"
#define ACTION_NAME_WWW "www"
#define ACTION_NAME_SEARCH "search"
#define ACTION_NAME_CONTROL_CENTER "controlCenter"

#define SCREENSAVER_DBUS_NAME "com.kylinsec.Kiran.ScreenSaver"
#define SCREENSAVER_DBUS_OBJECT_PATH "/com/kylinsec/Kiran/ScreenSaver"
#define SCREENSAVER_DBUS_INTERFACE "com.kylinsec.Kiran.ScreenSaver"

#define KSM_DBUS_NAME "org.gnome.SessionManager"
#define KSM_DBUS_OBJECT_PATH "/org/gnome/SessionManager"
#define KSM_DBUS_INTERFACE_NAME "org.gnome.SessionManager"

#define KIRAN_SHELL_DBUS_NAME "com.kylinsec.Kiran.Shell"
#define KIRAN_SHELL_MENU_DBUS_OBJECT_PATH "/com/kylinsec/Kiran/Shell/Menu"
#define KIRAN_SHELL_MENU_DBUS_INTERFACE "com.kylinsec.Kiran.Shell.Menu"

// 屏保锁屏后，检查锁屏状态的最大次数
#define SCREENSAVER_LOCK_CHECK_MAX_COUNT 50

KeysSystem::KeysSystem() : KeysComponent("System", tr("System")),
                           m_syncingFromSettings(false),
                           m_syncingFromKGlobalAccel(false)
{
    m_actionSchemaKeyMap.insert(ACTION_NAME_HELP, KEYS_SCHEMA_HELP);
    m_actionSchemaKeyMap.insert(ACTION_NAME_EMAIL, KEYS_SCHEMA_EMAIL);
    m_actionSchemaKeyMap.insert(ACTION_NAME_SCREENSAVER, KEYS_SCHEMA_SCREENSAVER);
    m_actionSchemaKeyMap.insert(ACTION_NAME_SHOW_DESKTOP, KEYS_SCHEMA_SHOW_DESKTOP);
    m_actionSchemaKeyMap.insert(ACTION_NAME_LOGOUT, KEYS_SCHEMA_LOGOUT);
    m_actionSchemaKeyMap.insert(ACTION_NAME_POWER, KEYS_SCHEMA_POWER);
    m_actionSchemaKeyMap.insert(ACTION_NAME_HOME, KEYS_SCHEMA_HOME);

    for (auto iter = m_actionSchemaKeyMap.constBegin(); iter != m_actionSchemaKeyMap.constEnd(); ++iter)
    {
        m_schemaActionMap.insert(iter.value(), iter.key());
    }
}

void KeysSystem::init()
{
    registerConfigurableShortcuts(false);

    registerShortCut(Qt::Key_Calculator, ACTION_NAME_CALCULATOR, tr("Launch calculator"));
    registerShortCut(Qt::Key_WWW, ACTION_NAME_WWW, tr("Launch web browser"));
    registerShortCut(Qt::Key_Search, ACTION_NAME_SEARCH, tr("Search"));
    registerShortCut(Qt::Key_Tools, ACTION_NAME_CONTROL_CENTER, tr("Launch settings"));

    // 开始菜单窗口是按下win键弹起时触发
    registerShortCut(QList<QKeySequence>() << Qt::Key_Super_L << Qt::Key_Super_R,
                     ACTION_NAME_START_MENU,
                     tr("Show start menu"),
                     false,
                     true);

    connect(m_settings, &QGSettings::changed, this, &KeysSystem::processSettingsChanged);
    connect(this, &KeysComponent::shortcutChanged, this, &KeysSystem::processKGlobalShortcutChanged);
}

void KeysSystem::registerConfigurableShortcuts(bool forceUpdate)
{
    for (auto iter = m_actionSchemaKeyMap.constBegin(); iter != m_actionSchemaKeyMap.constEnd(); ++iter)
    {
        registerConfigurableShortCut(iter.value(), forceUpdate);
    }
}

void KeysSystem::registerConfigurableShortCut(const QString &schemaKey, bool forceUpdate)
{
    auto actionName = actionBySchemaKey(schemaKey);
    RETURN_IF_TRUE(actionName.isEmpty());

    auto shortcut = m_settings->get(schemaKey).toString();
    registerShortCut(shortcut, actionName, displayNameByAction(actionName), true, forceUpdate);
}

void KeysSystem::processSettingsChanged(const QString &schemaKey)
{
    if (m_syncingFromKGlobalAccel)
    {
        return;
    }

    auto actionName = actionBySchemaKey(schemaKey);
    RETURN_IF_TRUE(actionName.isEmpty());

    auto shortcut = m_settings->get(schemaKey).toString();
    auto keySequenceList = KGlobalAccel::self()->globalShortcut(getComponentName(), actionName);
    auto currentShortcut = keySequenceList.isEmpty() ? QStringLiteral("") : keySequenceList.first().toString();
    RETURN_IF_TRUE(currentShortcut == shortcut);

    KLOG_INFO(keybinding) << "Syncing shortcut for"
                          << schemaKey << "from gsettings to KGlobalAccel, "
                          << "new shortcut is" << shortcut;

    /*目前m_syncingFromSettings变量的设计其实是多余的，只是为了多一层保险。实际测试中registerConfigurableShortCut
      调用的KGlobalAccel::self()->setShortcut函数是不会发送yourShortcutsChanged信号，不确定是kglobalaccel的bug还是设计如此。

      另外，就算会发送yourShortcutsChanged信号且是异步的，因为前面有RETURN_IF_TRUE(currentShortcut == shortcut);判断，
      所以不会出现循环同步的情况。*/
    m_syncingFromSettings = true;
    registerConfigurableShortCut(schemaKey, true);
    m_syncingFromSettings = false;
}

void KeysSystem::processKGlobalShortcutChanged(const QString &actionUnique)
{
    if (m_syncingFromSettings)
    {
        return;
    }

    auto schemaKey = schemaKeyByAction(actionUnique);
    if (schemaKey.isEmpty())
    {
        return;
    }

    auto keySequenceList = KGlobalAccel::self()->globalShortcut(getComponentName(), actionUnique);
    auto shortcut = keySequenceList.isEmpty() ? QStringLiteral("") : keySequenceList.first().toString();
    auto currentShortcut = m_settings->get(schemaKey).toString();
    RETURN_IF_TRUE(currentShortcut == shortcut);

    KLOG_INFO(keybinding) << "Syncing shortcut for"
                          << schemaKey << "from KGlobalAccel to gsettings, "
                          << "old shortcut is" << currentShortcut << "and "
                          << "new shortcut is" << shortcut;

    m_syncingFromKGlobalAccel = true;
    m_settings->set(schemaKey, shortcut);
    m_syncingFromKGlobalAccel = false;
}

QString KeysSystem::schemaKeyByAction(const QString &actionUnique) const
{
    return m_actionSchemaKeyMap.value(actionUnique);
}

QString KeysSystem::actionBySchemaKey(const QString &schemaKey) const
{
    return m_schemaActionMap.value(schemaKey);
}

QString KeysSystem::displayNameByAction(const QString &actionUnique) const
{
    switch (shash(actionUnique.toUtf8().data()))
    {
    case CONNECT(ACTION_NAME_HELP, _hash):
        return tr("Launch help browser");
    case CONNECT(ACTION_NAME_EMAIL, _hash):
        return tr("Launch email client");
    case CONNECT(ACTION_NAME_SCREENSAVER, _hash):
        return tr("Lock screen");
    case CONNECT(ACTION_NAME_SHOW_DESKTOP, _hash):
        return tr("Show desktop");
    case CONNECT(ACTION_NAME_LOGOUT, _hash):
        return tr("Log out");
    case CONNECT(ACTION_NAME_POWER, _hash):
        return tr("Shut down");
    case CONNECT(ACTION_NAME_HOME, _hash):
        return tr("Home folder");
    default:
        break;
    }

    return QString();
}

void KeysSystem::launchHelp()
{
    KDesktopFile desktopFile("user-manual.desktop");

    if (desktopFile.noDisplay())
    {
        KLOG_WARNING(keybinding) << "The user-manual.desktop is nodisplay, so ignore it.";
        return;
    }

    KService service(desktopFile.fileName());

    auto arguments = KIO::DesktopExecParser(service, QList<QUrl>()).resultingArguments();
    if (arguments.isEmpty())
    {
        KLOG_WARNING(keybinding) << "Failed to parse arguments for" << desktopFile.fileName();
        return;
    }

    auto program = arguments.takeFirst();
    QProcess::startDetached(program, arguments);
}

void KeysSystem::lockScreen()
{
    auto sendMessage = QDBusMessage::createMethodCall(SCREENSAVER_DBUS_NAME,
                                                      SCREENSAVER_DBUS_OBJECT_PATH,
                                                      SCREENSAVER_DBUS_INTERFACE,
                                                      "Lock");

    QDBusConnection::sessionBus().asyncCall(sendMessage);
}

void KeysSystem::showdesktop()
{
    KWindowSystem::setShowingDesktop(!KWindowSystem::showingDesktop());
}

void KeysSystem::popupStartMenu()
{
    auto sendMessage = QDBusMessage::createMethodCall(KIRAN_SHELL_DBUS_NAME,
                                                      KIRAN_SHELL_MENU_DBUS_OBJECT_PATH,
                                                      KIRAN_SHELL_MENU_DBUS_INTERFACE,
                                                      "activateStartMenu");

    QDBusConnection::sessionBus().asyncCall(sendMessage);
}

void KeysSystem::logout()
{
    auto sendMessage = QDBusMessage::createMethodCall(KSM_DBUS_NAME,
                                                      KSM_DBUS_OBJECT_PATH,
                                                      KSM_DBUS_INTERFACE_NAME,
                                                      "Logout");

    sendMessage << uint(0);

    QDBusConnection::sessionBus().asyncCall(sendMessage);
}

void KeysSystem::shutdown()
{
    auto sendMessage = QDBusMessage::createMethodCall(KSM_DBUS_NAME,
                                                      KSM_DBUS_OBJECT_PATH,
                                                      KSM_DBUS_INTERFACE_NAME,
                                                      "Shutdown");

    QDBusConnection::sessionBus().asyncCall(sendMessage);
}

void KeysSystem::openHome()
{
    auto homePath = QDir::homePath();
    QProcess::startDetached(QString("caja"), QStringList() << "--no-desktop" << homePath);
}

void KeysSystem::openCalculator()
{
    QProcess::startDetached("kiran-calculator", QStringList());
}

void KeysSystem::openSearch()
{
    QProcess::startDetached("mate-search-tool", QStringList());
}

void KeysSystem::openControlCenter()
{
    QProcess::startDetached("kiran-control-panel", QStringList());
}

void KeysSystem::triggerShortCut(const QString &name)
{
    KLOG_INFO(keybinding) << "Execute command for shortcut" << name;

    switch (shash(name.toLatin1().data()))
    {
    case CONNECT(ACTION_NAME_HELP, _hash):
        launchHelp();
        break;
    case CONNECT(ACTION_NAME_EMAIL, _hash):
    {
        auto emailApps = XdgDefaultApps::emailClient();
        if (emailApps)
        {
            emailApps->startDetached();
        }
        break;
    }
    case CONNECT(ACTION_NAME_SCREENSAVER, _hash):
        lockScreen();
        break;
    case CONNECT(ACTION_NAME_SHOW_DESKTOP, _hash):
        showdesktop();
        break;
    case CONNECT(ACTION_NAME_START_MENU, _hash):
        popupStartMenu();
        break;
    case CONNECT(ACTION_NAME_LOGOUT, _hash):
        logout();
        break;
    case CONNECT(ACTION_NAME_POWER, _hash):
        shutdown();
        break;
    case CONNECT(ACTION_NAME_HOME, _hash):
        openHome();
        break;
    case CONNECT(ACTION_NAME_CALCULATOR, _hash):
        openCalculator();
        break;
    case CONNECT(ACTION_NAME_WWW, _hash):
    {
        auto webApps = XdgDefaultApps::webBrowser();
        if (webApps)
        {
            webApps->startDetached();
        }
        break;
    }
    case CONNECT(ACTION_NAME_SEARCH, _hash):
        openSearch();
        break;
    case CONNECT(ACTION_NAME_CONTROL_CENTER, _hash):
        openControlCenter();
        break;
    default:
        break;
    }
}

}  // namespace Kiran