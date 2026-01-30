/**
 * Copyright (c) 2026 ~ 2027 KylinSec Co., Ltd.
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

#include "test-appearance.h"
#include <QDBusConnection>
#include "appearance-i.h"
#include "plugins/appearance/appearance-plugin.h"
#include "test/appearance/appearance_interface.h"

namespace Kiran
{

void TestAppearance::initTestCase()
{
    qputenv("GSETTINGS_BACKEND", "memory");

    m_plugin = new AppearancePlugin();
    m_plugin->activate();
    m_appearanceProxy = new AppearanceProxy(APPEARANCE_DBUS_NAME,
                                            APPEARANCE_OBJECT_PATH,
                                            QDBusConnection::sessionBus(),
                                            this);
}

void TestAppearance::cleanupTestCase()
{
    m_plugin->deactivate();
    delete m_plugin;
    m_plugin = nullptr;
}

void TestAppearance::testSetTheme()
{
    // 设置GTK主题
    m_appearanceProxy->SetTheme(AppearanceThemeType::APPEARANCE_THEME_TYPE_GTK, "Kiran-white").waitForFinished();
    auto newTheme = m_appearanceProxy->GetTheme(AppearanceThemeType::APPEARANCE_THEME_TYPE_GTK);
    QVERIFY(newTheme == "Kiran-white");

    // 设置METACITY主题
    m_appearanceProxy->SetTheme(AppearanceThemeType::APPEARANCE_THEME_TYPE_METACITY, "Kiran-white").waitForFinished();
    newTheme = m_appearanceProxy->GetTheme(AppearanceThemeType::APPEARANCE_THEME_TYPE_METACITY);
    QVERIFY(newTheme == "Kiran-white");

    // 设置ICON主题
    m_appearanceProxy->SetTheme(AppearanceThemeType::APPEARANCE_THEME_TYPE_ICON, "Summer").waitForFinished();
    newTheme = m_appearanceProxy->GetTheme(AppearanceThemeType::APPEARANCE_THEME_TYPE_ICON);
    QVERIFY(newTheme == "Summer");

    // 设置CURSOR主题
    m_appearanceProxy->SetTheme(AppearanceThemeType::APPEARANCE_THEME_TYPE_CURSOR, "Kiran").waitForFinished();
    newTheme = m_appearanceProxy->GetTheme(AppearanceThemeType::APPEARANCE_THEME_TYPE_CURSOR);
    QVERIFY(newTheme == "Kiran");
}

void TestAppearance::testSetFont()
{
    m_appearanceProxy->SetFont(AppearanceFontType::APPEARANCE_FONT_TYPE_APPLICATION, "Noto Sans CJK SC 10").waitForFinished();
    auto newFont = m_appearanceProxy->GetFont(AppearanceFontType::APPEARANCE_FONT_TYPE_APPLICATION);
    QVERIFY(newFont == "Noto Sans CJK SC 10");

    m_appearanceProxy->SetFont(AppearanceFontType::APPEARANCE_FONT_TYPE_DOCUMENT, "Noto Sans CJK SC 11").waitForFinished();
    newFont = m_appearanceProxy->GetFont(AppearanceFontType::APPEARANCE_FONT_TYPE_DOCUMENT);
    QVERIFY(newFont == "Noto Sans CJK SC 11");

    m_appearanceProxy->SetFont(AppearanceFontType::APPEARANCE_FONT_TYPE_DESKTOP, "Noto Sans CJK SC 12").waitForFinished();
    newFont = m_appearanceProxy->GetFont(AppearanceFontType::APPEARANCE_FONT_TYPE_DESKTOP);
    QVERIFY(newFont == "Noto Sans CJK SC 12");

    m_appearanceProxy->SetFont(AppearanceFontType::APPEARANCE_FONT_TYPE_WINDOW_TITLE, "Noto Sans CJK SC 13").waitForFinished();
    newFont = m_appearanceProxy->GetFont(AppearanceFontType::APPEARANCE_FONT_TYPE_WINDOW_TITLE);
    QVERIFY(newFont == "Noto Sans CJK SC 13");

    m_appearanceProxy->SetFont(AppearanceFontType::APPEARANCE_FONT_TYPE_MONOSPACE, "Monospace 12").waitForFinished();
    newFont = m_appearanceProxy->GetFont(AppearanceFontType::APPEARANCE_FONT_TYPE_MONOSPACE);
    QVERIFY(newFont == "Monospace 12");
}

void TestAppearance::testBackground()
{
    // 设置桌面背景
    m_appearanceProxy->SetDesktopBackground("test.jpg").waitForFinished();
    auto newBackground = m_appearanceProxy->desktop_background();
    QVERIFY(newBackground == "test.jpg");

    // 设置锁屏背景
    m_appearanceProxy->SetLockScreenBackground("test.jpg").waitForFinished();
    auto newLockScreenBackground = m_appearanceProxy->lock_screen_background();
    QVERIFY(newLockScreenBackground == "test.jpg");
}
}  // namespace Kiran

QTEST_MAIN(Kiran::TestAppearance)