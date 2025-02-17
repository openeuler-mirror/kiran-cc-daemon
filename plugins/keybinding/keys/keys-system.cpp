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
#include "../keybinding-utils.h"
#include "keybinding-i.h"
#include "lib/base/base.h"

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

KeysSystem::KeysSystem() : KeysComponent("System", tr("System"))
{
}

void KeysSystem::init()
{
    auto helpKey = m_settings->get(KEYS_SCHEMA_HELP).toString();
    auto emailKey = m_settings->get(KEYS_SCHEMA_EMAIL).toString();
    auto screensaverKey = m_settings->get(KEYS_SCHEMA_SCREENSAVER).toString();
    auto showdesktopKey = m_settings->get(KEYS_SCHEMA_SHOW_DESKTOP).toString();
    auto logoutKey = m_settings->get(KEYS_SCHEMA_LOGOUT).toString();
    auto powerKey = m_settings->get(KEYS_SCHEMA_POWER).toString();
    auto homeKey = m_settings->get(KEYS_SCHEMA_HOME).toString();

    registerShortCut(KeybindingUtils::keyCombGtk2Qt(helpKey), ACTION_NAME_HELP, tr("Launch help browser"));
    registerShortCut(KeybindingUtils::keyCombGtk2Qt(emailKey), ACTION_NAME_EMAIL, tr("Launch email client"));
    registerShortCut(KeybindingUtils::keyCombGtk2Qt(screensaverKey), ACTION_NAME_SCREENSAVER, tr("Lock screen"));
    registerShortCut(KeybindingUtils::keyCombGtk2Qt(showdesktopKey), ACTION_NAME_SHOW_DESKTOP, tr("Show desktop"));
    // 开始菜单窗口是按下win键弹起时触发
    registerShortCut(Qt::Key_Super_L, ACTION_NAME_START_MENU, tr("Show start menu"), false);
    registerShortCut(KeybindingUtils::keyCombGtk2Qt(logoutKey), ACTION_NAME_LOGOUT, tr("Log out"));
    registerShortCut(KeybindingUtils::keyCombGtk2Qt(powerKey), ACTION_NAME_POWER, tr("Shut down"));
    registerShortCut(KeybindingUtils::keyCombGtk2Qt(homeKey), ACTION_NAME_HOME, tr("Home folder"));

    registerShortCut(Qt::Key_Calculator, ACTION_NAME_CALCULATOR, tr("Launch calculator"));
    registerShortCut(Qt::Key_WWW, ACTION_NAME_WWW, tr("Launch web browser"));
    registerShortCut(Qt::Key_Search, ACTION_NAME_SEARCH, tr("Search"));
    registerShortCut(Qt::Key_Tools, ACTION_NAME_CONTROL_CENTER, tr("Launch settings"));
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
        KLOG_WARNING(keybinding) << "Failed to parse arguments for " << desktopFile.fileName();
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