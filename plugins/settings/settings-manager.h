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
#include <QMap>
#include "dbus-types.h"

class SettingsAdaptor;
class QGSettings;
class QTimer;

namespace Kiran
{
class RegistryXSettings;
class RegistryGnome;
class SettingsXResource;

class SettingsManager : public QObject,
                        protected QDBusContext
{
    Q_OBJECT

    Q_PROPERTY(bool AutomaticMnemonics READ getAutomaticMnemonics)
    Q_PROPERTY(bool ButtonImages READ getButtonImages)
    Q_PROPERTY(QString ColorScheme READ getColorScheme)
    Q_PROPERTY(bool CursorBlink READ getCursorBlink)
    Q_PROPERTY(int CursorBlinkTime READ getCursorBlinkTime)
    Q_PROPERTY(int CursorBlinkTimeout READ getCursorBlinkTimeout)
    Q_PROPERTY(QString CursorThemeName READ getCursorThemeName)
    Q_PROPERTY(int CursorThemeSize READ getCursorThemeSize)
    Q_PROPERTY(QString DecorationLayout READ getDecorationLayout)
    Q_PROPERTY(bool DialogsUseHeader READ getDialogsUseHeader)
    Q_PROPERTY(int DndDragThreshold READ getDndDragThreshold)
    Q_PROPERTY(int DoubleClickDistance READ getDoubleClickDistance)
    Q_PROPERTY(int DoubleClickTime READ getDoubleClickTime)
    Q_PROPERTY(bool EnableAnimations READ getEnableAnimations)
    Q_PROPERTY(bool EnableEventSounds READ getEnableEventSounds)
    Q_PROPERTY(bool EnableInputFeedbackSounds READ getEnableInputFeedbackSounds)
    Q_PROPERTY(bool EnablePrimaryPaste READ getEnablePrimaryPaste)
    Q_PROPERTY(QString FileChooserBackend READ getFileChooserBackend)
    Q_PROPERTY(double FontDPI READ getFontDPI)
    Q_PROPERTY(QString FontName READ getFontName)
    Q_PROPERTY(QString IconThemeName READ getIconThemeName)
    Q_PROPERTY(QString ImModule READ getImModule)
    Q_PROPERTY(QString ImPreeditStyle READ getImPreeditStyle)
    Q_PROPERTY(QString ImStatusStyle READ getImStatusStyle)
    Q_PROPERTY(QString KeyThemeName READ getKeyThemeName)
    Q_PROPERTY(bool MenuImages READ getMenuImages)
    Q_PROPERTY(QString MenubarAccel READ getMenubarAccel)
    Q_PROPERTY(bool ShellShowsAppMenu READ getShellShowsAppMenu)
    Q_PROPERTY(bool ShellShowsMenubar READ getShellShowsMenubar)
    Q_PROPERTY(bool ShowInputMethodMenu READ getShowInputMethodMenu)
    Q_PROPERTY(bool ShowUnicodeMenu READ getShowUnicodeMenu)
    Q_PROPERTY(QString SoundThemeName READ getSoundThemeName)
    Q_PROPERTY(QString ThemeName READ getThemeName)
    Q_PROPERTY(QString ToolbarIconsSize READ getToolbarIconsSize)
    Q_PROPERTY(QString ToolbarStyle READ getToolbarStyle)
    Q_PROPERTY(int WindowScalingFactor READ getWindowScalingFactor)
    Q_PROPERTY(int XftAntialias READ getXftAntialias)
    Q_PROPERTY(int XftDPI READ getXftDPI)
    Q_PROPERTY(QString XftHintStyle READ getXftHintStyle)
    Q_PROPERTY(int XftHinting READ getXftHinting)
    Q_PROPERTY(QString XftRgba READ getXftRGBA)

public:
    SettingsManager();
    virtual ~SettingsManager();

    static SettingsManager *getInstance() { return m_instance; };
    static void globalInit();
    static void globalDeinit() { delete m_instance; };
    int getWindowScale();

public:
    bool getAutomaticMnemonics() const;
    bool getButtonImages() const;
    QString getColorScheme() const;
    bool getCursorBlink() const;
    int getCursorBlinkTime() const;
    int getCursorBlinkTimeout() const;
    QString getCursorThemeName() const;
    int getCursorThemeSize() const;
    QString getDecorationLayout() const;
    bool getDialogsUseHeader() const;
    int getDndDragThreshold() const;
    int getDoubleClickDistance() const;
    int getDoubleClickTime() const;
    bool getEnableAnimations() const;
    bool getEnableEventSounds() const;
    bool getEnableInputFeedbackSounds() const;
    bool getEnablePrimaryPaste() const;
    QString getFileChooserBackend() const;
    double getFontDPI() const;
    QString getFontName() const;
    QString getIconThemeName() const;
    QString getImModule() const;
    QString getImPreeditStyle() const;
    QString getImStatusStyle() const;
    QString getKeyThemeName() const;
    bool getMenuImages() const;
    QString getMenubarAccel() const;
    bool getReloadWhenScaling() const;
    bool getShellShowsAppMenu() const;
    bool getShellShowsMenubar() const;
    bool getShowInputMethodMenu() const;
    bool getShowUnicodeMenu() const;
    QString getSoundThemeName() const;
    QString getThemeName() const;
    QString getToolbarIconsSize() const;
    QString getToolbarStyle() const;
    int getWindowScalingFactor() const;
    int getXftAntialias() const;
    int getXftDPI() const;
    QString getXftHintStyle() const;
    int getXftHinting() const;
    QString getXftRGBA() const;

public Q_SLOTS:  // METHODS
    void SetAutomaticMnemonics(bool automaticMnemonics);
    void SetButtonImages(bool buttonImages);
    void SetColorScheme(const QString &colorScheme);
    void SetCursorBlink(bool cursorBlink);
    void SetCursorBlinkTime(int cursorBlinkTime);
    void SetCursorBlinkTimeout(int cursorBlinkTimeout);
    void SetCursorThemeName(const QString &cursorThemeName);
    void SetCursorThemeSize(int cursorThemeSize);
    void SetDecorationLayout(const QString &decorationLayout);
    void SetDialogsUseHeader(bool dialogsUseHeader);
    void SetDndDragThreshold(int dndDragThreshold);
    void SetDoubleClickDistance(int doubleClickDistance);
    void SetDoubleClickTime(int doubleClickTime);
    void SetEnableAnimations(bool enableAnimations);
    void SetEnableEventSounds(bool enableEventSounds);
    void SetEnableInputFeedbackSounds(bool enableInputFeedbackSounds);
    void SetEnablePrimaryPaste(bool enablePrimaryPaste);
    void SetFileChooserBackend(const QString &fileChooserBackend);
    void SetFontDPI(double fontDpi);
    void SetFontName(const QString &fontName);
    void SetIconThemeName(const QString &iconThemeName);
    void SetImModule(const QString &imModule);
    void SetImPreeditStyle(const QString &imPreeditStyle);
    void SetImStatusStyle(const QString &imStatusStyle);
    void SetKeyThemeName(const QString &keyThemeName);
    void SetMenuImages(bool menuImages);
    void SetMenubarAccel(const QString &menubarAccel);
    void SetShellShowsAppMenu(bool shellShowsAppMenu);
    void SetShellShowsMenubar(bool shellShowsMenubar);
    void SetShowInputMethodMenu(bool showInputMethodMenu);
    void SetShowUnicodeMenu(bool showUnicodeMenu);
    void SetSoundThemeName(const QString &soundThemeName);
    void SetThemeName(const QString &themeName);
    void SetToolbarIconsSize(const QString &toolbarIconsSize);
    void SetToolbarStyle(const QString &toolbarStyle);
    void SetWindowScalingFactor(int windowScalingFactor);
    void SetXftAntialias(int xftAntialias);
    void SetXftDPI(int xftDpi);
    void SetXftHintStyle(const QString &xftHintStyle);
    void SetXftHinting(int xftHinting);
    void SetXftRGBA(const QString &xftRgba);

public:
    bool getWindowScalingFactorQtSync();
    double getOptimizeDPI();

private:
    void init();

    void settingsChanged(const QString &key);
    void scaleSettings();
    void scaleChangeWorkarounds(int32_t scale);
    void processScreenChanged();
    void enableShowDesktopIcon();

private:
    static SettingsManager *m_instance;
    SettingsAdaptor *m_settingsAdaptor;

    // 当window_scaling_factor_为0时，根据屏幕信息设置缩放，否则跟window_scaling_factor_相同。
    int32_t m_windowScale;

    QGSettings *m_settings;
    QGSettings *m_backgroundSettings;
    RegistryXSettings *m_registryXsettings;
    RegistryGnome *m_registryGnome;
    SettingsXResource *m_xresource;

    QMap<QString, QString> m_registry2Schema;

    QTimer *m_showDesktopIconTimer;
};
}  // namespace Kiran