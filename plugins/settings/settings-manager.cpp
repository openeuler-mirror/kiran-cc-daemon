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

#include "settings-manager.h"
#include <QDBusConnection>
#include <QGSettings>
#include <QGuiApplication>
#include <QProcess>
#include <QScreen>
#include <QTimer>
#include "lib/xcb/EWMH.h"
#include "registry/registry-gnome.h"
#include "registry/registry-xsettings.h"
#include "settings-common.h"
#include "settings-i.h"
#include "settings-utils.h"
#include "settings-xresource.h"
#include "settingsadaptor.h"

namespace Kiran
{
#define BACKGROUND_SCHAME_ID "org.mate.background"
#define BACKGROUND_SCHEMA_SHOW_DESKTOP_ICONS "showDesktopIcons"

SettingsManager::SettingsManager() : m_windowScale(0),
                                     m_registryXsettings(nullptr),
                                     m_xresource(nullptr)
{
    m_settingsAdaptor = new SettingsAdaptor(this);
    m_settings = new QGSettings(SETTINGS_SCHEMA_ID, "", this);
    m_backgroundSettings = new QGSettings(BACKGROUND_SCHAME_ID, "", this);
    if (qGuiApp->platformName() == "xcb")
    {
        m_registryXsettings = new RegistryXSettings(this);
        m_xresource = new SettingsXResource(this);
    }
    m_registryGnome = new RegistryGnome(this);
    m_showDesktopIconTimer = new QTimer(this);
}

SettingsManager::~SettingsManager()
{
}

SettingsManager *SettingsManager::m_instance = nullptr;

void SettingsManager::globalInit()
{
    m_instance = new SettingsManager();
    m_instance->init();
}

int SettingsManager::getWindowScale()
{
    auto scale = getWindowScalingFactor();
    if (!scale)
    {
        scale = SettingsUtils::getWindowScaleAuto();
    }
    return scale;
}

#define CHECK_VAR(var, type, retval)                                                              \
    if (!var)                                                                                     \
    {                                                                                             \
        DBUS_ERROR_REPLY_AND_RETVAL(retval, CCErrorCode::ERROR_XSETTINGS_NOTFOUND_PROPERTY);      \
    }                                                                                             \
    if (var->getType() != type)                                                                   \
    {                                                                                             \
        DBUS_ERROR_REPLY_AND_RETVAL(retval, CCErrorCode::ERROR_XSETTINGS_PROPERTY_TYPE_MISMATCH); \
    }

bool SettingsManager::getAutomaticMnemonics() const
{
    return m_settings->get(SETTINGS_SCHEMA_GTK_AUTOMATIC_MNEMONICS).toBool();
}

bool SettingsManager::getButtonImages() const
{
    return m_settings->get(SETTINGS_SCHEMA_GTK_BUTTON_IMAGES).toBool();
}

QString SettingsManager::getColorScheme() const
{
    return m_settings->get(SETTINGS_SCHEMA_GTK_COLOR_SCHEME).toString();
}

bool SettingsManager::getCursorBlink() const
{
    return m_settings->get(SETTINGS_SCHEMA_NET_CURSOR_BLINK).toBool();
}

int SettingsManager::getCursorBlinkTime() const
{
    return m_settings->get(SETTINGS_SCHEMA_NET_CURSOR_BLINK_TIME).toInt();
}

int SettingsManager::getCursorBlinkTimeout() const
{
    return m_settings->get(SETTINGS_SCHEMA_GTK_CURSOR_BLINK_TIMEOUT).toInt();
}

QString SettingsManager::getCursorThemeName() const
{
    return m_settings->get(SETTINGS_SCHEMA_GTK_CURSOR_THEME_NAME).toString();
}

int SettingsManager::getCursorThemeSize() const
{
    return m_settings->get(SETTINGS_SCHEMA_GTK_CURSOR_THEME_SIZE).toInt();
}

QString SettingsManager::getDecorationLayout() const
{
    return m_settings->get(SETTINGS_SCHEMA_GTK_DECORATION_LAYOUT).toString();
}

bool SettingsManager::getDialogsUseHeader() const
{
    return m_settings->get(SETTINGS_SCHEMA_GTK_DIALOGS_USE_HEADER).toBool();
}

int SettingsManager::getDndDragThreshold() const
{
    return m_settings->get(SETTINGS_SCHEMA_NET_DND_DRAG_THRESHOLD).toInt();
}

int SettingsManager::getDoubleClickDistance() const
{
    return m_settings->get(SETTINGS_SCHEMA_NET_DOUBLE_CLICK_DISTANCE).toInt();
}

int SettingsManager::getDoubleClickTime() const
{
    return m_settings->get(SETTINGS_SCHEMA_NET_DOUBLE_CLICK_TIME).toInt();
}

bool SettingsManager::getEnableAnimations() const
{
    return m_settings->get(SETTINGS_SCHEMA_GTK_ENABLE_ANIMATIONS).toBool();
}

bool SettingsManager::getEnableEventSounds() const
{
    return m_settings->get(SETTINGS_SCHEMA_NET_ENABLE_EVENT_SOUNDS).toBool();
}

bool SettingsManager::getEnableInputFeedbackSounds() const
{
    return m_settings->get(SETTINGS_SCHEMA_NET_ENABLE_INPUT_FEEDBACK_SOUNDS).toBool();
}

bool SettingsManager::getEnablePrimaryPaste() const
{
    return m_settings->get(SETTINGS_SCHEMA_GTK_ENABLE_PRIMARY_PASTE).toBool();
}

QString SettingsManager::getFileChooserBackend() const
{
    return m_settings->get(SETTINGS_SCHEMA_GTK_FILE_CHOOSER_BACKEND).toString();
}

double SettingsManager::getFontDPI() const
{
    return m_settings->get(SETTINGS_SCHEMA_FONT_DPI).toDouble();
}

QString SettingsManager::getFontName() const
{
    return m_settings->get(SETTINGS_SCHEMA_GTK_FONT_NAME).toString();
}

QString SettingsManager::getIconThemeName() const
{
    return m_settings->get(SETTINGS_SCHEMA_NET_ICON_THEME_NAME).toString();
}

QString SettingsManager::getImModule() const
{
    return m_settings->get(SETTINGS_SCHEMA_GTK_IM_MODULE).toString();
}

QString SettingsManager::getImPreeditStyle() const
{
    return m_settings->get(SETTINGS_SCHEMA_GTK_IM_PREEDIT_STYLE).toString();
}

QString SettingsManager::getImStatusStyle() const
{
    return m_settings->get(SETTINGS_SCHEMA_GTK_IM_STATUS_STYLE).toString();
}

QString SettingsManager::getKeyThemeName() const
{
    return m_settings->get(SETTINGS_SCHEMA_GTK_KEY_THEME_NAME).toString();
}

bool SettingsManager::getMenuImages() const
{
    return m_settings->get(SETTINGS_SCHEMA_GTK_MENU_IMAGES).toBool();
}

QString SettingsManager::getMenubarAccel() const
{
    return m_settings->get(SETTINGS_SCHEMA_GTK_MENUBAR_ACCEL).toString();
}

bool SettingsManager::getShellShowsAppMenu() const
{
    return m_settings->get(SETTINGS_SCHEMA_GTK_SHELL_SHOWS_APP_MENU).toBool();
}

bool SettingsManager::getShellShowsMenubar() const
{
    return m_settings->get(SETTINGS_SCHEMA_GTK_SHELL_SHOWS_MENUBAR).toBool();
}

bool SettingsManager::getShowInputMethodMenu() const
{
    return m_settings->get(SETTINGS_SCHEMA_GTK_SHOW_INPUT_METHOD_MENU).toBool();
}

bool SettingsManager::getShowUnicodeMenu() const
{
    return m_settings->get(SETTINGS_SCHEMA_GTK_SHOW_UNICODE_MENU).toBool();
}

QString SettingsManager::getSoundThemeName() const
{
    return m_settings->get(SETTINGS_SCHEMA_NET_SOUND_THEME_NAME).toString();
}

QString SettingsManager::getThemeName() const
{
    return m_settings->get(SETTINGS_SCHEMA_NET_THEME_NAME).toString();
}

QString SettingsManager::getToolbarIconsSize() const
{
    return m_settings->get(SETTINGS_SCHEMA_GTK_TOOLBAR_ICONS_SIZE).toString();
}

QString SettingsManager::getToolbarStyle() const
{
    return m_settings->get(SETTINGS_SCHEMA_GTK_TOOLBAR_STYLE).toString();
}

int SettingsManager::getWindowScalingFactor() const
{
    return m_settings->get(SETTINGS_SCHEMA_WINDOW_SCALING_FACTOR).toInt();
}

int SettingsManager::getXftAntialias() const
{
    return m_settings->get(SETTINGS_SCHEMA_XFT_ANTIALIAS).toInt();
}

int SettingsManager::getXftDPI() const
{
    return m_settings->get(SETTINGS_SCHEMA_XFT_DPI).toInt();
}

QString SettingsManager::getXftHintStyle() const
{
    return m_settings->get(SETTINGS_SCHEMA_XFT_HINT_STYLE).toString();
}

int SettingsManager::getXftHinting() const
{
    return m_settings->get(SETTINGS_SCHEMA_XFT_HINTING).toInt();
}

QString SettingsManager::getXftRGBA() const
{
    return m_settings->get(SETTINGS_SCHEMA_XFT_RGBA).toString();
}

void SettingsManager::SetAutomaticMnemonics(bool automaticMnemonics)
{
    m_settings->set(SETTINGS_SCHEMA_GTK_AUTOMATIC_MNEMONICS, automaticMnemonics);
}

void SettingsManager::SetButtonImages(bool buttonImages)
{
    m_settings->set(SETTINGS_SCHEMA_GTK_BUTTON_IMAGES, buttonImages);
}

void SettingsManager::SetColorScheme(const QString &colorScheme)
{
    m_settings->set(SETTINGS_SCHEMA_GTK_COLOR_SCHEME, colorScheme);
}

void SettingsManager::SetCursorBlink(bool cursorBlink)
{
    m_settings->set(SETTINGS_SCHEMA_NET_CURSOR_BLINK, cursorBlink);
}

void SettingsManager::SetCursorBlinkTime(int cursorBlinkTime)
{
    m_settings->set(SETTINGS_SCHEMA_NET_CURSOR_BLINK_TIME, cursorBlinkTime);
}

void SettingsManager::SetCursorBlinkTimeout(int cursorBlinkTimeout)
{
    m_settings->set(SETTINGS_SCHEMA_GTK_CURSOR_BLINK_TIMEOUT, cursorBlinkTimeout);
}

void SettingsManager::SetCursorThemeName(const QString &cursorThemeName)
{
    m_settings->set(SETTINGS_SCHEMA_GTK_CURSOR_THEME_NAME, cursorThemeName);
}

void SettingsManager::SetCursorThemeSize(int cursorThemeSize)
{
    m_settings->set(SETTINGS_SCHEMA_GTK_CURSOR_THEME_SIZE, cursorThemeSize);
}

void SettingsManager::SetDecorationLayout(const QString &decorationLayout)
{
    m_settings->set(SETTINGS_SCHEMA_GTK_DECORATION_LAYOUT, decorationLayout);
}

void SettingsManager::SetDialogsUseHeader(bool dialogsUseHeader)
{
    m_settings->set(SETTINGS_SCHEMA_GTK_DIALOGS_USE_HEADER, dialogsUseHeader);
}

void SettingsManager::SetDndDragThreshold(int dndDragThreshold)
{
    m_settings->set(SETTINGS_SCHEMA_NET_DND_DRAG_THRESHOLD, dndDragThreshold);
}

void SettingsManager::SetDoubleClickDistance(int doubleClickDistance)
{
    m_settings->set(SETTINGS_SCHEMA_NET_DOUBLE_CLICK_DISTANCE, doubleClickDistance);
}

void SettingsManager::SetDoubleClickTime(int doubleClickTime)
{
    m_settings->set(SETTINGS_SCHEMA_NET_DOUBLE_CLICK_TIME, doubleClickTime);
}

void SettingsManager::SetEnableAnimations(bool enableAnimations)
{
    m_settings->set(SETTINGS_SCHEMA_GTK_ENABLE_ANIMATIONS, enableAnimations);
}

void SettingsManager::SetEnableEventSounds(bool enableEventSounds)
{
    m_settings->set(SETTINGS_SCHEMA_NET_ENABLE_EVENT_SOUNDS, enableEventSounds);
}

void SettingsManager::SetEnableInputFeedbackSounds(bool enableInputFeedbackSounds)
{
    m_settings->set(SETTINGS_SCHEMA_NET_ENABLE_INPUT_FEEDBACK_SOUNDS, enableInputFeedbackSounds);
}

void SettingsManager::SetEnablePrimaryPaste(bool enablePrimaryPaste)
{
    m_settings->set(SETTINGS_SCHEMA_GTK_ENABLE_PRIMARY_PASTE, enablePrimaryPaste);
}

void SettingsManager::SetFileChooserBackend(const QString &fileChooserBackend)
{
    m_settings->set(SETTINGS_SCHEMA_GTK_FILE_CHOOSER_BACKEND, fileChooserBackend);
}

void SettingsManager::SetFontDPI(double fontDpi)
{
    m_settings->set(SETTINGS_SCHEMA_FONT_DPI, fontDpi);
}

void SettingsManager::SetFontName(const QString &fontName)
{
    m_settings->set(SETTINGS_SCHEMA_GTK_FONT_NAME, fontName);
}

void SettingsManager::SetIconThemeName(const QString &iconThemeName)
{
    m_settings->set(SETTINGS_SCHEMA_NET_ICON_THEME_NAME, iconThemeName);
}

void SettingsManager::SetImModule(const QString &imModule)
{
    m_settings->set(SETTINGS_SCHEMA_GTK_IM_MODULE, imModule);
}

void SettingsManager::SetImPreeditStyle(const QString &imPreeditStyle)
{
    m_settings->set(SETTINGS_SCHEMA_GTK_IM_PREEDIT_STYLE, imPreeditStyle);
}

void SettingsManager::SetImStatusStyle(const QString &imStatusStyle)
{
    m_settings->set(SETTINGS_SCHEMA_GTK_IM_STATUS_STYLE, imStatusStyle);
}

void SettingsManager::SetKeyThemeName(const QString &keyThemeName)
{
    m_settings->set(SETTINGS_SCHEMA_GTK_KEY_THEME_NAME, keyThemeName);
}

void SettingsManager::SetMenuImages(bool menuImages)
{
    m_settings->set(SETTINGS_SCHEMA_GTK_MENU_IMAGES, menuImages);
}

void SettingsManager::SetMenubarAccel(const QString &menubarAccel)
{
    m_settings->set(SETTINGS_SCHEMA_GTK_MENUBAR_ACCEL, menubarAccel);
}

void SettingsManager::SetShellShowsAppMenu(bool shellShowsAppMenu)
{
    m_settings->set(SETTINGS_SCHEMA_GTK_SHELL_SHOWS_APP_MENU, shellShowsAppMenu);
}

void SettingsManager::SetShellShowsMenubar(bool shellShowsMenubar)
{
    m_settings->set(SETTINGS_SCHEMA_GTK_SHELL_SHOWS_MENUBAR, shellShowsMenubar);
}

void SettingsManager::SetShowInputMethodMenu(bool showInputMethodMenu)
{
    m_settings->set(SETTINGS_SCHEMA_GTK_SHOW_INPUT_METHOD_MENU, showInputMethodMenu);
}

void SettingsManager::SetShowUnicodeMenu(bool showUnicodeMenu)
{
    m_settings->set(SETTINGS_SCHEMA_GTK_SHOW_UNICODE_MENU, showUnicodeMenu);
}

void SettingsManager::SetSoundThemeName(const QString &soundThemeName)
{
    m_settings->set(SETTINGS_SCHEMA_NET_SOUND_THEME_NAME, soundThemeName);
}

void SettingsManager::SetThemeName(const QString &themeName)
{
    m_settings->set(SETTINGS_SCHEMA_NET_THEME_NAME, themeName);
}

void SettingsManager::SetToolbarIconsSize(const QString &toolbarIconsSize)
{
    m_settings->set(SETTINGS_SCHEMA_GTK_TOOLBAR_ICONS_SIZE, toolbarIconsSize);
}

void SettingsManager::SetToolbarStyle(const QString &toolbarStyle)
{
    m_settings->set(SETTINGS_SCHEMA_GTK_TOOLBAR_STYLE, toolbarStyle);
}

void SettingsManager::SetWindowScalingFactor(int windowScalingFactor)
{
    m_settings->set(SETTINGS_SCHEMA_WINDOW_SCALING_FACTOR, windowScalingFactor);
}

void SettingsManager::SetXftAntialias(int xftAntialias)
{
    m_settings->set(SETTINGS_SCHEMA_XFT_ANTIALIAS, xftAntialias);
}

void SettingsManager::SetXftDPI(int xftDpi)
{
    m_settings->set(SETTINGS_SCHEMA_XFT_DPI, xftDpi);
}

void SettingsManager::SetXftHintStyle(const QString &xftHintStyle)
{
    m_settings->set(SETTINGS_SCHEMA_XFT_HINT_STYLE, xftHintStyle);
}

void SettingsManager::SetXftHinting(int xftHinting)
{
    m_settings->set(SETTINGS_SCHEMA_XFT_HINTING, xftHinting);
}

void SettingsManager::SetXftRGBA(const QString &xftRgba)
{
    m_settings->set(SETTINGS_SCHEMA_XFT_RGBA, xftRgba);
}

bool SettingsManager::getWindowScalingFactorQtSync()
{
    return m_settings->get(SETTINGS_SCHEMA_WINDOW_SCALING_FACTOR_QT_SYNC).toBool();
}

double SettingsManager::getOptimizeDPI()
{
    double dpi = getFontDPI();
    if (dpi < EPS)
    {
        dpi = SettingsUtils::getDPIFromXServer();
    }
    return dpi;
}

void SettingsManager::init()
{
    auto primaryScreen = QGuiApplication::primaryScreen();

    scaleSettings();

    if (m_registryXsettings)
    {
        m_registryXsettings->init();
        m_xresource->init();
    }
    m_registryGnome->init();

    connect(m_settings, &QGSettings::changed, this, &SettingsManager::settingsChanged);
    connect(primaryScreen, &QScreen::virtualGeometryChanged, this, &SettingsManager::processScreenChanged);
    connect(m_showDesktopIconTimer, &QTimer::timeout, this, &SettingsManager::enableShowDesktopIcon);

    auto sessionConnection = QDBusConnection::sessionBus();
    if (!sessionConnection.registerService(SETTINGS_DBUS_NAME))
    {
        KLOG_WARNING() << "Failed to register dbus name:" << SETTINGS_DBUS_NAME;
        return;
    }

    if (!sessionConnection.registerObject(SETTINGS_OBJECT_PATH, SETTINGS_DBUS_INTERFACE_NAME, this))
    {
        KLOG_ERROR() << "Can't register object:" << sessionConnection.lastError();
        return;
    }
}

void SettingsManager::settingsChanged(const QString &key)
{
    switch (shash(key.toLatin1().data()))
    {
    case CONNECT(SETTINGS_SCHEMA_WINDOW_SCALING_FACTOR, _hash):
    case CONNECT(SETTINGS_SCHEMA_GTK_CURSOR_THEME_SIZE, _hash):
    case CONNECT(SETTINGS_SCHEMA_FONT_DPI, _hash):
        scaleSettings();
        break;
    default:
        break;
    }
}

void SettingsManager::scaleSettings()
{
    auto scale = getWindowScale();
    auto dpi = getOptimizeDPI();
    int scaledDPI = int(SettingsUtils::formatScaleDPI(scale, dpi) * 1024);

    SetXftDPI(scaledDPI);
    scaleChangeWorkarounds(scale);
}

void SettingsManager::scaleChangeWorkarounds(int32_t scale)
{
    KLOG_DEBUG(settings) << "window scale is" << m_windowScale << ", scale is" << scale;

    bool isInit = (!m_windowScale);

    RETURN_IF_TRUE(m_windowScale == scale);
    m_windowScale = scale;

    /* 第一次初始化时缩放率是没有变化的，所以不应该重启底部面板、文件管理器和窗口管理器，
    这样会导致进入会话时出现屏幕刷新的视觉效果，而且底部面板和文件管理器崩溃的概率较大*/

    // 如果开启QT缩放同步，则将缩放值同步到QT缩放相关的环境变量
    if (getWindowScalingFactorQtSync())
    {
        if (!SettingsUtils::updateUserEnvVariable("QT_AUTO_SCREEN_SCALE_FACTOR", "0"))
        {
            KLOG_WARNING(settings) << "There was a problem when setting QT_AUTO_SCREEN_SCALE_FACTOR=0";
        }

        /* FIXME: 由于QT_SCALE_FACTOR将会放大窗口以及pt大小字体，而缩放将会更改Xft.dpi属性，该属性也会导致qt pt字体大小放大，字体将会放大过多。
            目前暂时解决方案：缩放两倍时固定Qt字体DPI 96，由QT_SCALE_FACTOR环境变量对窗口以及字体进行放大.
            后续应弃用QT_SCALE_FACTOR方案
            */
        if (!SettingsUtils::updateUserEnvVariable("QT_SCALE_FACTOR", scale == 2 ? "2" : "1"))
        {
            KLOG_WARNING(settings) << "There was a problem when setting QT_SCALE_FACTOR=" << scale;
        }
        else if (scale == 2 && !SettingsUtils::updateUserEnvVariable("QT_FONT_DPI", "96"))
        {
            KLOG_WARNING(settings) << "There was a problem when setting QT_FONT_DPI=96";
        }
    }

    auto reloadWhenScaling = m_settings->get(SETTINGS_SCHEMA_RELOAD_WHEN_SCALING).toBool();

    if (!isInit && reloadWhenScaling)
    {
        // 理想的情况是marco/mate-panel/caja监控缩放因子的变化而自动调整自己的大小，
        // 但实际上没有实现这个功能，所以当窗口缩放因子发生变化时重置它们

        /* 重启marco窗口管理器，因为缩放率是在下次进入会话时生效，所以这个逻辑可能是在会话刚启动时被调用，
           此时marco可能还未启动，获取的wmName为空。*/
        QString wmName;
        if (qGuiApp->platformName() == QLatin1String("xcb"))
        {
            wmName = EWMH::getDefault()->getWmName();
        }

        if (wmName == WM_COMMON_MARCO)
        {
            if (!QProcess::startDetached("marco", QStringList{"--replace"}))
            {
                KLOG_WARNING(settings) << "There was a problem restarting marco";
            }
            else
            {
                KLOG_INFO(settings) << "Restart marco successfully";
            }
        }

        // 重启面板
        if (!QProcess::startDetached("killall", QStringList{"mate-panel", "kiran-panel", "kiran-shell"}))
        {
            KLOG_WARNING(settings) << "There was a problem restarting mate-panel, kiran-panel and kiran-shell.";
        }
        else
        {
            KLOG_INFO(settings) << "Restart panel successfully";
        }

        // 重置桌面图标大小
        if (m_backgroundSettings &&
            m_backgroundSettings->get(BACKGROUND_SCHEMA_SHOW_DESKTOP_ICONS).toBool() &&
            !m_showDesktopIconTimer->isActive())
        {
            KLOG_INFO(settings) << "Disable show-desktop-icon properties then enable to repaint the desktop icon.";
            m_backgroundSettings->set(BACKGROUND_SCHEMA_SHOW_DESKTOP_ICONS, false);
            // 延时显示桌面图标，给文件管理器一定的时间重绘
            m_showDesktopIconTimer->start(1000);
        }
    }
}

void SettingsManager::processScreenChanged()
{
    auto scale = getWindowScale();
    if (scale != m_windowScale)
    {
        scaleSettings();
    }
}

void SettingsManager::enableShowDesktopIcon()
{
    if (m_backgroundSettings)
    {
        m_backgroundSettings->set(BACKGROUND_SCHEMA_SHOW_DESKTOP_ICONS, true);
    }
    m_showDesktopIconTimer->stop();
}

}  // namespace Kiran