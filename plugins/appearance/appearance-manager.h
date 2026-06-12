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
#include "theme/theme-data.h"

class QGSettings;
class AppearanceAdaptor;

namespace Kiran
{
class AppearanceTheme;
class AppearanceFont;
class AppearanceBackground;

class AppearanceManager : public QObject,
                          protected QDBusContext
{
    Q_OBJECT

    Q_PROPERTY(bool AutoSwitchWindowTheme READ getAutoSwitchWindowTheme)
    Q_PROPERTY(QString desktop_background READ getDesktopBackground WRITE SetDesktopBackground)
    Q_PROPERTY(QString lock_screen_background READ getLockScreenBackground WRITE SetLockScreenBackground)

public:
    AppearanceManager();
    virtual ~AppearanceManager();

    static AppearanceManager* getInstance() { return m_instance; };

    static void globalInit();
    static void globalDeinit() { delete m_instance; };

public:
    bool getAutoSwitchWindowTheme() const;
    QString getDesktopBackground() const { return m_desktopBackground; };
    QString getLockScreenBackground() const { return m_lockScreenBackground; };

public Q_SLOTS:
    // 开启窗口主题自动切换后，在启动程序时会根据当前时间来选择一个主题。
    void EnableAutoSwitchWindowTheme();
    // 获取类型为type的字体，字体包括了字体名和字体大小(注意：其他项目的命名规范可能是字体名已经包含了字体大小，可能存在术语不统一的情况）。
    // 例如字体'San 10'的字体名为'San'，字体大小为10，type分类可参考AppearanceFontType
    QString GetFont(int type);
    // 获取指定类型的主题名
    QString GetTheme(int type);
    // 获取特定类型(参考AppearanceThemeType)的主题，返回的是一个json格式的字符串，方便后续做字段兼容性扩展
    QString GetThemes(int type);
    // 获取光标大小
    int GetCursorSize();
    // 重置默认字体
    void ResetFont(int type);
    // 设置桌面背景
    void SetDesktopBackground(const QString& desktopBackground);
    // 设置类型为type的字体
    void SetFont(int type, const QString& font);
    // 设置锁屏背景
    void SetLockScreenBackground(const QString& lockScreenBackground);
    // 将type类型的主题设置为theme_name。
    void SetTheme(int type, const QString& themeName);
    // 设置光标大小
    void SetCursorSize(int size);
Q_SIGNALS:  // SIGNALS
    void FontChanged(int type, const QString& font);
    void ThemeChanged(int type, const QString& themeName);
    void CursorSizeChanged(int size);

private:
    void init();
    void loadFromSettings();

    // 开启主题自动切换
    void autoSwitchForWindowTheme();
    void NotifyThemeChanged(ThemeKey themeKey);
    void NotifyFontChanged(AppearanceFontType type, const QString& font);
    void NotifyCursorSizeChanged(int size);
    void processSettingsChanged(const QString& key);

private:
    static AppearanceManager* m_instance;

    QGSettings* m_appearanceSettings;
    AppearanceAdaptor* m_appearanceAdaptor;

    AppearanceTheme* m_appearanceTheme;
    AppearanceFont* m_appearanceFont;
    AppearanceBackground* m_appearanceBackground;

    QString m_desktopBackground;
    QString m_lockScreenBackground;
};
}  // namespace Kiran
