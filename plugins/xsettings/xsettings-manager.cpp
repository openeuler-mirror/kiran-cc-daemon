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

#include "xsettings-manager.h"
#include <QDBusConnection>
#include <QGSettings>
#include <QGuiApplication>
#include <QProcess>
#include <QScreen>
#include <QTimer>
#include "fontconfig-monitor.h"
#include "lib/xcb/EWMH.h"
#include "xsettings-common.h"
#include "xsettings-i.h"
#include "xsettings-registry.h"
#include "xsettings-utils.h"
#include "xsettings-xresource.h"
#include "xsettingsadaptor.h"

namespace Kiran
{
#define BACKGROUND_SCHAME_ID "org.mate.background"
#define BACKGROUND_SCHEMA_SHOW_DESKTOP_ICONS "showDesktopIcons"

const QMap<QString, QString> XSettingsManager::m_schema2Registry =
    {
        {XSETTINGS_SCHEMA_NET_DOUBLE_CLICK_TIME, XSETTINGS_REGISTRY_PROP_NET_DOUBLE_CLICK_TIME},
        {XSETTINGS_SCHEMA_NET_DOUBLE_CLICK_DISTANCE, XSETTINGS_REGISTRY_PROP_NET_DOUBLE_CLICK_DISTANCE},
        {XSETTINGS_SCHEMA_NET_DND_DRAG_THRESHOLD, XSETTINGS_REGISTRY_PROP_NET_DND_DRAG_THRESHOLD},
        {XSETTINGS_SCHEMA_NET_CURSOR_BLINK, XSETTINGS_REGISTRY_PROP_NET_CURSOR_BLINK},
        {XSETTINGS_SCHEMA_NET_CURSOR_BLINK_TIME, XSETTINGS_REGISTRY_PROP_NET_CURSOR_BLINK_TIME},
        {XSETTINGS_SCHEMA_NET_THEME_NAME, XSETTINGS_REGISTRY_PROP_NET_THEME_NAME},
        {XSETTINGS_SCHEMA_NET_ICON_THEME_NAME, XSETTINGS_REGISTRY_PROP_NET_ICON_THEME_NAME},
        {XSETTINGS_SCHEMA_NET_ENABLE_EVENT_SOUNDS, XSETTINGS_REGISTRY_PROP_NET_ENABLE_EVENT_SOUNDS},
        {XSETTINGS_SCHEMA_NET_SOUND_THEME_NAME, XSETTINGS_REGISTRY_PROP_NET_SOUND_THEME_NAME},
        {XSETTINGS_SCHEMA_NET_ENABLE_INPUT_FEEDBACK_SOUNDS, XSETTINGS_REGISTRY_PROP_NET_ENABLE_INPUT_FEEDBACK_SOUNDS},

        {XSETTINGS_SCHEMA_XFT_ANTIALIAS, XSETTINGS_REGISTRY_PROP_XFT_ANTIALIAS},
        {XSETTINGS_SCHEMA_XFT_HINTING, XSETTINGS_REGISTRY_PROP_XFT_HINTING},
        {XSETTINGS_SCHEMA_XFT_HINT_STYLE, XSETTINGS_REGISTRY_PROP_XFT_HINT_STYLE},
        {XSETTINGS_SCHEMA_XFT_RGBA, XSETTINGS_REGISTRY_PROP_XFT_RGBA},
        {XSETTINGS_SCHEMA_XFT_DPI, XSETTINGS_REGISTRY_PROP_XFT_DPI},

        {XSETTINGS_SCHEMA_GTK_CURSOR_THEME_NAME, XSETTINGS_REGISTRY_PROP_GTK_CURSOR_THEME_NAME},
        {XSETTINGS_SCHEMA_GTK_CURSOR_THEME_SIZE, XSETTINGS_REGISTRY_PROP_GTK_CURSOR_THEME_SIZE},
        {XSETTINGS_SCHEMA_GTK_FONT_NAME, XSETTINGS_REGISTRY_PROP_GTK_FONT_NAME},
        {XSETTINGS_SCHEMA_GTK_KEY_THEME_NAME, XSETTINGS_REGISTRY_PROP_GTK_KEY_THEME_NAME},
        {XSETTINGS_SCHEMA_GTK_TOOLBAR_STYLE, XSETTINGS_REGISTRY_PROP_GTK_TOOLBAR_STYLE},
        {XSETTINGS_SCHEMA_GTK_TOOLBAR_ICONS_SIZE, XSETTINGS_REGISTRY_PROP_GTK_TOOLBAR_ICON_SIZE},
        {XSETTINGS_SCHEMA_GTK_IM_PREEDIT_STYLE, XSETTINGS_REGISTRY_PROP_GTK_IM_PREEDIT_STYLE},
        {XSETTINGS_SCHEMA_GTK_IM_STATUS_STYLE, XSETTINGS_REGISTRY_PROP_GTK_IM_STATUS_STYLE},
        {XSETTINGS_SCHEMA_GTK_IM_MODULE, XSETTINGS_REGISTRY_PROP_GTK_IM_MODULE},
        {XSETTINGS_SCHEMA_GTK_MENU_IMAGES, XSETTINGS_REGISTRY_PROP_GTK_MENU_IMAGES},
        {XSETTINGS_SCHEMA_GTK_BUTTON_IMAGES, XSETTINGS_REGISTRY_PROP_GTK_BUTTON_IMAGES},
        {XSETTINGS_SCHEMA_GTK_MENUBAR_ACCEL, XSETTINGS_REGISTRY_PROP_GTK_MENU_BAR_ACCEL},
        {XSETTINGS_SCHEMA_GTK_COLOR_SCHEME, XSETTINGS_REGISTRY_PROP_GTK_COLOR_SCHEME},
        {XSETTINGS_SCHEMA_GTK_FILE_CHOOSER_BACKEND, XSETTINGS_REGISTRY_PROP_GTK_FILE_CHOOSER_BACKEND},
        {XSETTINGS_SCHEMA_GTK_DECORATION_LAYOUT, XSETTINGS_REGISTRY_PROP_GTK_DECORATION_LAYOUT},
        {XSETTINGS_SCHEMA_GTK_SHELL_SHOWS_APP_MENU, XSETTINGS_REGISTRY_PROP_GTK_SHELL_SHOWS_APP_MENU},
        {XSETTINGS_SCHEMA_GTK_SHELL_SHOWS_MENUBAR, XSETTINGS_REGISTRY_PROP_GTK_SHELL_SHOWS_MENUBAR},
        {XSETTINGS_SCHEMA_GTK_SHOW_INPUT_METHOD_MENU, XSETTINGS_REGISTRY_PROP_GTK_SHOW_INPUT_METHOD_MENU},
        {XSETTINGS_SCHEMA_GTK_SHOW_UNICODE_MENU, XSETTINGS_REGISTRY_PROP_GTK_SHOW_UNICODE_MENU},
        {XSETTINGS_SCHEMA_GTK_AUTOMATIC_MNEMONICS, XSETTINGS_REGISTRY_PROP_GTK_AUTO_MNEMONICS},
        {XSETTINGS_SCHEMA_GTK_ENABLE_PRIMARY_PASTE, XSETTINGS_REGISTRY_PROP_GTK_ENABLE_PRIMARY_PASTE},
        {XSETTINGS_SCHEMA_GTK_ENABLE_ANIMATIONS, XSETTINGS_REGISTRY_PROP_GTK_ENABLE_ANIMATIONS},
        {XSETTINGS_SCHEMA_GTK_DIALOGS_USE_HEADER, XSETTINGS_REGISTRY_PROP_GTK_DIALOGS_USE_HEADER},

        {XSETTINGS_SCHEMA_WINDOW_SCALING_FACTOR, XSETTINGS_REGISTRY_PROP_GDK_WINDOW_SCALING_FACTOR},
};

XSettingsManager::XSettingsManager() : m_windowScale(0)
{
    m_xsettingsAdaptor = new XSettingsAdaptor(this);
    m_xsettingsSettings = new QGSettings(XSETTINGS_SCHEMA_ID, "", this);
    m_backgroundSettings = new QGSettings(BACKGROUND_SCHAME_ID, "", this);
    m_registry = new XSettingsRegistry(this);
    m_xresource = new XSettingsXResource(this);
    m_fontconfigMonitor = new FontconfigMonitor(this);
    m_showDesktopIconTimer = new QTimer(this);

    for (const auto &key : XSettingsManager::m_schema2Registry)
    {
        m_registry2Schema.insert(XSettingsManager::m_schema2Registry[key], key);
    }
}

XSettingsManager::~XSettingsManager()
{
}

XSettingsManager *XSettingsManager::m_instance = nullptr;

void XSettingsManager::globalInit()
{
    m_instance = new XSettingsManager();
    m_instance->init();
}

int XSettingsManager::getWindowScale()
{
    auto scale = getWindowScalingFactor();
    if (!scale)
    {
        scale = XSettingsUtils::getWindowScaleAuto();
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

DColor XSettingsManager::GetColor(const QString &name)
{
    auto var = m_registry->getProperty(name);

    CHECK_VAR(var, XSettingsPropType::XSETTINGS_PROP_TYPE_COLOR, DColor());

    auto colorVar = qSharedPointerCast<XSettingsPropertyColor>(var);
    auto colorValue = colorVar->getValue();
    return DColor(colorValue.red, colorValue.green, colorValue.blue, colorValue.alpha);
}

int XSettingsManager::GetInteger(const QString &name)
{
    auto var = m_registry->getProperty(name);
    CHECK_VAR(var, XSettingsPropType::XSETTINGS_PROP_TYPE_INT, 0);

    auto intVar = qSharedPointerCast<XSettingsPropertyInt>(var);
    return intVar->getValue();
}

QString XSettingsManager::GetString(const QString &name)
{
    auto var = m_registry->getProperty(name);
    CHECK_VAR(var, XSettingsPropType::XSETTINGS_PROP_TYPE_STRING, QString());

    auto stringVar = qSharedPointerCast<XSettingsPropertyString>(var);
    return stringVar->getValue();
}

QStringList XSettingsManager::ListPropertyNames()
{
    QStringList propertyNames;
    auto properties = m_registry->getProperties();
    for (auto &iter : properties)
    {
        propertyNames.push_back(iter->getName());
    }
    return propertyNames;
}

void XSettingsManager::SetColor(const QString &name, DColor value)
{
    // 暂无颜色属性
    DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_XSETTINGS_PROPERTY_UNSUPPORTED);
}

void XSettingsManager::SetInteger(const QString &name, int value)
{
    auto iter = m_registry2Schema.find(name);
    if (iter == m_registry2Schema.end())
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_XSETTINGS_PROPERTY_INVALID);
    }

    switch (shash(name.toLatin1().data()))
    {
    case CONNECT(XSETTINGS_REGISTRY_PROP_NET_DOUBLE_CLICK_TIME, _hash):
    case CONNECT(XSETTINGS_REGISTRY_PROP_NET_DOUBLE_CLICK_DISTANCE, _hash):
    case CONNECT(XSETTINGS_REGISTRY_PROP_NET_DND_DRAG_THRESHOLD, _hash):
    case CONNECT(XSETTINGS_REGISTRY_PROP_NET_CURSOR_BLINK_TIME, _hash):
    case CONNECT(XSETTINGS_REGISTRY_PROP_XFT_ANTIALIAS, _hash):
    case CONNECT(XSETTINGS_REGISTRY_PROP_XFT_HINTING, _hash):
        m_xsettingsSettings->set(iter.value(), value);
        break;
    case CONNECT(XSETTINGS_REGISTRY_PROP_NET_CURSOR_BLINK, _hash):
    case CONNECT(XSETTINGS_REGISTRY_PROP_NET_ENABLE_EVENT_SOUNDS, _hash):
    case CONNECT(XSETTINGS_REGISTRY_PROP_NET_ENABLE_INPUT_FEEDBACK_SOUNDS, _hash):
    case CONNECT(XSETTINGS_REGISTRY_PROP_GTK_MENU_IMAGES, _hash):
    case CONNECT(XSETTINGS_REGISTRY_PROP_GTK_BUTTON_IMAGES, _hash):
    case CONNECT(XSETTINGS_REGISTRY_PROP_GTK_SHELL_SHOWS_APP_MENU, _hash):
    case CONNECT(XSETTINGS_REGISTRY_PROP_GTK_SHELL_SHOWS_MENUBAR, _hash):
    case CONNECT(XSETTINGS_REGISTRY_PROP_GTK_SHOW_INPUT_METHOD_MENU, _hash):
    case CONNECT(XSETTINGS_REGISTRY_PROP_GTK_SHOW_UNICODE_MENU, _hash):
    case CONNECT(XSETTINGS_REGISTRY_PROP_GTK_AUTO_MNEMONICS, _hash):
    case CONNECT(XSETTINGS_REGISTRY_PROP_GTK_ENABLE_PRIMARY_PASTE, _hash):
    case CONNECT(XSETTINGS_REGISTRY_PROP_GTK_ENABLE_ANIMATIONS, _hash):
    case CONNECT(XSETTINGS_REGISTRY_PROP_GTK_DIALOGS_USE_HEADER, _hash):
        m_xsettingsSettings->set(iter.value(), bool(value));
        break;
    default:
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_XSETTINGS_PROPERTY_UNSUPPORTED);
        break;
    }
}

void XSettingsManager::SetString(const QString &name, const QString &value)
{
    auto iter = m_registry2Schema.find(name);
    if (iter == m_registry2Schema.end())
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_XSETTINGS_PROPERTY_INVALID);
    }

    switch (shash(name.toLatin1().data()))
    {
    case CONNECT(XSETTINGS_REGISTRY_PROP_NET_THEME_NAME, _hash):
    case CONNECT(XSETTINGS_REGISTRY_PROP_NET_ICON_THEME_NAME, _hash):
    case CONNECT(XSETTINGS_REGISTRY_PROP_NET_SOUND_THEME_NAME, _hash):
    case CONNECT(XSETTINGS_REGISTRY_PROP_XFT_HINT_STYLE, _hash):
    case CONNECT(XSETTINGS_REGISTRY_PROP_XFT_RGBA, _hash):
    case CONNECT(XSETTINGS_REGISTRY_PROP_GTK_CURSOR_THEME_NAME, _hash):
    case CONNECT(XSETTINGS_REGISTRY_PROP_GTK_FONT_NAME, _hash):
    case CONNECT(XSETTINGS_REGISTRY_PROP_GTK_KEY_THEME_NAME, _hash):
    case CONNECT(XSETTINGS_REGISTRY_PROP_GTK_TOOLBAR_STYLE, _hash):
    case CONNECT(XSETTINGS_REGISTRY_PROP_GTK_TOOLBAR_ICON_SIZE, _hash):
    case CONNECT(XSETTINGS_REGISTRY_PROP_GTK_IM_PREEDIT_STYLE, _hash):
    case CONNECT(XSETTINGS_REGISTRY_PROP_GTK_IM_STATUS_STYLE, _hash):
    case CONNECT(XSETTINGS_REGISTRY_PROP_GTK_IM_MODULE, _hash):
    case CONNECT(XSETTINGS_REGISTRY_PROP_GTK_MENU_BAR_ACCEL, _hash):
    case CONNECT(XSETTINGS_REGISTRY_PROP_GTK_COLOR_SCHEME, _hash):
    case CONNECT(XSETTINGS_REGISTRY_PROP_GTK_FILE_CHOOSER_BACKEND, _hash):
    case CONNECT(XSETTINGS_REGISTRY_PROP_GTK_DECORATION_LAYOUT, _hash):
        m_xsettingsSettings->set(iter.value(), value);
        break;
    default:
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_XSETTINGS_PROPERTY_UNSUPPORTED);
        break;
    }
}

int XSettingsManager::getXftAntialias()
{
    return m_xsettingsSettings->get(XSETTINGS_SCHEMA_XFT_ANTIALIAS).toInt();
}

int XSettingsManager::getXftHinting()
{
    return m_xsettingsSettings->get(XSETTINGS_SCHEMA_XFT_HINTING).toInt();
}

QString XSettingsManager::getXftHintStyle()
{
    return m_xsettingsSettings->get(XSETTINGS_SCHEMA_XFT_HINT_STYLE).toString();
}

QString XSettingsManager::getXftRGBA()
{
    return m_xsettingsSettings->get(XSETTINGS_SCHEMA_XFT_RGBA).toString();
}

int XSettingsManager::getXftDPI()
{
    return m_xsettingsSettings->get(XSETTINGS_SCHEMA_XFT_DPI).toInt();
}

double XSettingsManager::getFontDPI()
{
    return m_xsettingsSettings->get(XSETTINGS_SCHEMA_FONT_DPI).toDouble();
}
QString XSettingsManager::getGtkCursorThemeName()
{
    return m_xsettingsSettings->get(XSETTINGS_SCHEMA_GTK_CURSOR_THEME_NAME).toString();
}
int XSettingsManager::getGtkCursorThemeSize()
{
    return m_xsettingsSettings->get(XSETTINGS_SCHEMA_GTK_CURSOR_THEME_SIZE).toInt();
}
int XSettingsManager::getWindowScalingFactor()
{
    return m_xsettingsSettings->get(XSETTINGS_SCHEMA_WINDOW_SCALING_FACTOR).toInt();
}
bool XSettingsManager::getWindowScalingFactorQtSync()
{
    return m_xsettingsSettings->get(XSETTINGS_SCHEMA_WINDOW_SCALING_FACTOR_QT_SYNC).toBool();
}

void XSettingsManager::init()
{
    auto primaryScreen = QGuiApplication::primaryScreen();

    RETURN_IF_FALSE(m_xsettingsSettings);
    RETURN_IF_FALSE(m_registry->init());
    m_fontconfigMonitor->init();
    loadFromSettings();

    m_xresource->init();

    connect(m_xsettingsSettings, &QGSettings::changed, std::bind(&XSettingsManager::settingsChanged, this, std::placeholders::_1, true));
    connect(primaryScreen, &QScreen::virtualGeometryChanged, this, &XSettingsManager::processScreenChanged);
    connect(m_fontconfigMonitor, &FontconfigMonitor::timestampChanged, this, &XSettingsManager::processFontconfigTimestampChanged);
    connect(m_registry, &XSettingsRegistry::propertiesChanged, this, &XSettingsManager::processPropertiesChanged);
    connect(m_showDesktopIconTimer, &QTimer::timeout, this, &XSettingsManager::enableShowDesktopIcon);

    auto sessionConnection = QDBusConnection::sessionBus();
    if (!sessionConnection.registerService(XSETTINGS_DBUS_NAME))
    {
        KLOG_WARNING() << "Failed to register dbus name: " << XSETTINGS_DBUS_NAME;
        return;
    }

    if (!sessionConnection.registerObject(XSETTINGS_OBJECT_PATH, XSETTINGS_DBUS_INTERFACE_NAME, this))
    {
        KLOG_ERROR() << "Can't register object:" << sessionConnection.lastError();
        return;
    }
}

void XSettingsManager::loadFromSettings()
{
    for (const auto &key : m_xsettingsSettings->keys())
    {
        // 这里不做通知，等初始化完后统一通知
        settingsChanged(key, false);
    }
}

void XSettingsManager::settingsChanged(const QString &key, bool isNotify)
{
    if (isNotify)
    {
        KLOG_DEBUG(xsettings) << "The" << key << "settings changed.";
    }

    auto iter = m_schema2Registry.find(key);

#define SET_CASE(prop, type)                                   \
    case CONNECT(prop, _hash):                                 \
    {                                                          \
        auto value = m_xsettingsSettings->get(key).to##type(); \
        m_registry->update(iter.value(), value);               \
        break;                                                 \
    }

    switch (shash(key.toLatin1().data()))
    {
        SET_CASE(XSETTINGS_SCHEMA_NET_DOUBLE_CLICK_TIME, Int);
        SET_CASE(XSETTINGS_SCHEMA_NET_DOUBLE_CLICK_DISTANCE, Int);
        SET_CASE(XSETTINGS_SCHEMA_NET_DND_DRAG_THRESHOLD, Int);
        SET_CASE(XSETTINGS_SCHEMA_NET_CURSOR_BLINK, Bool);
        SET_CASE(XSETTINGS_SCHEMA_NET_CURSOR_BLINK_TIME, Int);
        SET_CASE(XSETTINGS_SCHEMA_NET_THEME_NAME, String);
        SET_CASE(XSETTINGS_SCHEMA_NET_ICON_THEME_NAME, String);
        SET_CASE(XSETTINGS_SCHEMA_NET_ENABLE_EVENT_SOUNDS, Bool);
        SET_CASE(XSETTINGS_SCHEMA_NET_SOUND_THEME_NAME, String);
        SET_CASE(XSETTINGS_SCHEMA_NET_ENABLE_INPUT_FEEDBACK_SOUNDS, Bool);

        SET_CASE(XSETTINGS_SCHEMA_XFT_ANTIALIAS, Int);
        SET_CASE(XSETTINGS_SCHEMA_XFT_HINTING, Int);
        SET_CASE(XSETTINGS_SCHEMA_XFT_HINT_STYLE, String);
        SET_CASE(XSETTINGS_SCHEMA_XFT_RGBA, String);

        SET_CASE(XSETTINGS_SCHEMA_GTK_CURSOR_THEME_NAME, String);
        SET_CASE(XSETTINGS_SCHEMA_GTK_FONT_NAME, String);
        SET_CASE(XSETTINGS_SCHEMA_GTK_KEY_THEME_NAME, String);
        SET_CASE(XSETTINGS_SCHEMA_GTK_TOOLBAR_STYLE, String);
        SET_CASE(XSETTINGS_SCHEMA_GTK_TOOLBAR_ICONS_SIZE, String);
        SET_CASE(XSETTINGS_SCHEMA_GTK_IM_PREEDIT_STYLE, String);
        SET_CASE(XSETTINGS_SCHEMA_GTK_IM_STATUS_STYLE, String);
        SET_CASE(XSETTINGS_SCHEMA_GTK_IM_MODULE, String);
        SET_CASE(XSETTINGS_SCHEMA_GTK_MENU_IMAGES, Bool);
        SET_CASE(XSETTINGS_SCHEMA_GTK_BUTTON_IMAGES, Bool);
        SET_CASE(XSETTINGS_SCHEMA_GTK_MENUBAR_ACCEL, String);
        SET_CASE(XSETTINGS_SCHEMA_GTK_COLOR_SCHEME, String);
        SET_CASE(XSETTINGS_SCHEMA_GTK_FILE_CHOOSER_BACKEND, String);
        SET_CASE(XSETTINGS_SCHEMA_GTK_DECORATION_LAYOUT, String);
        SET_CASE(XSETTINGS_SCHEMA_GTK_SHELL_SHOWS_APP_MENU, Bool);
        SET_CASE(XSETTINGS_SCHEMA_GTK_SHELL_SHOWS_MENUBAR, Bool);
        SET_CASE(XSETTINGS_SCHEMA_GTK_SHOW_INPUT_METHOD_MENU, Bool);
        SET_CASE(XSETTINGS_SCHEMA_GTK_SHOW_UNICODE_MENU, Bool);
        SET_CASE(XSETTINGS_SCHEMA_GTK_AUTOMATIC_MNEMONICS, Bool);
        SET_CASE(XSETTINGS_SCHEMA_GTK_ENABLE_PRIMARY_PASTE, Bool);
        SET_CASE(XSETTINGS_SCHEMA_GTK_ENABLE_ANIMATIONS, Bool);
        SET_CASE(XSETTINGS_SCHEMA_GTK_DIALOGS_USE_HEADER, Bool);

        // Ignore these properties
    case CONNECT(XSETTINGS_SCHEMA_GTK_CURSOR_THEME_SIZE, _hash):
    case CONNECT(XSETTINGS_SCHEMA_WINDOW_SCALING_FACTOR, _hash):
    case CONNECT(XSETTINGS_SCHEMA_WINDOW_SCALING_FACTOR_QT_SYNC, _hash):
    case CONNECT(XSETTINGS_SCHEMA_XFT_DPI, _hash):
    case CONNECT(XSETTINGS_SCHEMA_FONT_DPI, _hash):
        break;

    default:
        KLOG_WARNING(xsettings) << "Unknown key" << key;
        break;
    }
#undef SET_CASET

    switch (shash(key.toLatin1().data()))
    {
    case CONNECT(XSETTINGS_SCHEMA_WINDOW_SCALING_FACTOR, _hash):
    case CONNECT(XSETTINGS_SCHEMA_GTK_CURSOR_THEME_SIZE, _hash):
    case CONNECT(XSETTINGS_SCHEMA_FONT_DPI, _hash):
        scaleSettings();
        break;
    case CONNECT(XSETTINGS_SCHEMA_XFT_RGBA, _hash):
        m_registry->update(XSETTINGS_REGISTRY_PROP_XFT_LCDFILTER, getXftRGBA() == "rgb" ? "lcddefault" : "none");
        break;
    default:
        break;
    }

    m_registry->update(XSETTINGS_REGISTRY_PROP_NET_FALLBACK_ICON_THEME, "mate");

    if (isNotify)
    {
        Q_EMIT xsettingsChanged(key);
    }
}

double XSettingsManager::getOptimizeDPI()
{
    double dpi = getFontDPI();
    if (dpi < EPS)
    {
        dpi = XSettingsUtils::getDPIFromXServer();
    }
    return dpi;
}

void XSettingsManager::scaleSettings()
{
    auto scale = getWindowScale();
    auto dpi = getOptimizeDPI();
    int unscaledDPI = int(dpi * 1024);
    int scaledDPI = int(XSettingsUtils::formatScaleDPI(scale, dpi) * 1024);
    auto scaledCursorSize = getGtkCursorThemeSize() * scale;

    m_registry->update(XSETTINGS_REGISTRY_PROP_GDK_WINDOW_SCALING_FACTOR, scale);
    m_registry->update(XSETTINGS_REGISTRY_PROP_GDK_UNSCALED_DPI, unscaledDPI);
    m_registry->update(XSETTINGS_REGISTRY_PROP_XFT_DPI, scaledDPI);
    m_registry->update(XSETTINGS_REGISTRY_PROP_GTK_CURSOR_THEME_SIZE, scaledCursorSize);

    m_xsettingsSettings->set(XSETTINGS_SCHEMA_XFT_DPI, scaledDPI);
    scaleChangeWorkarounds(scale);
}

void XSettingsManager::scaleChangeWorkarounds(int32_t scale)
{
    KLOG_DEBUG(xsettings) << "window scale is " << m_windowScale << ", scale is" << scale;

    bool isInit = (!m_windowScale);

    RETURN_IF_TRUE(m_windowScale == scale);
    m_windowScale = scale;

    /* 第一次初始化时缩放率是没有变化的，所以不应该重启底部面板、文件管理器和窗口管理器，
    这样会导致进入会话时出现屏幕刷新的视觉效果，而且底部面板和文件管理器崩溃的概率较大*/

    // 如果开启QT缩放同步，则将缩放值同步到QT缩放相关的环境变量
    if (getWindowScalingFactorQtSync())
    {
        if (!XSettingsUtils::updateUserEnvVariable("QT_AUTO_SCREEN_SCALE_FACTOR", "0"))
        {
            KLOG_WARNING(xsettings) << "There was a problem when setting QT_AUTO_SCREEN_SCALE_FACTOR=0";
        }

        /* FIXME: 由于QT_SCALE_FACTOR将会放大窗口以及pt大小字体，而缩放将会更改Xft.dpi属性，该属性也会导致qt pt字体大小放大，字体将会放大过多。
            目前暂时解决方案：缩放两倍时固定Qt字体DPI 96，由QT_SCALE_FACTOR环境变量对窗口以及字体进行放大.
            后续应弃用QT_SCALE_FACTOR方案
            */
        if (!XSettingsUtils::updateUserEnvVariable("QT_SCALE_FACTOR", scale == 2 ? "2" : "1"))
        {
            KLOG_WARNING(xsettings) << "There was a problem when setting QT_SCALE_FACTOR=" << scale;
        }
        else if (scale == 2 && !XSettingsUtils::updateUserEnvVariable("QT_FONT_DPI", "96"))
        {
            KLOG_WARNING(xsettings) << "There was a problem when setting QT_FONT_DPI=96";
        }
    }

    if (!isInit)
    {
        // 理想的情况是marco/mate-panel/caja监控缩放因子的变化而自动调整自己的大小，
        // 但实际上没有实现这个功能，所以当窗口缩放因子发生变化时重置它们

        /* 重启marco窗口管理器，因为缩放率是在下次进入会话时生效，所以这个逻辑可能是在会话刚启动时被调用，
           此时marco可能还未启动，获取的wmName为空。*/
        auto wmName = EWMH::getDefault()->getWmName();
        if (wmName == WM_COMMON_MARCO)
        {
            if (!QProcess::startDetached("marco", QStringList{"--replace"}))
            {
                KLOG_WARNING(xsettings) << "There was a problem restarting marco";
            }
            else
            {
                KLOG_INFO(xsettings) << "Restart marco successfully";
            }
        }

        // 重启面板
        if (!QProcess::startDetached("killall", QStringList{"mate-panel", "kiran-panel"}))
        {
            KLOG_WARNING(xsettings) << "There was a problem restarting mate-panel or kiran-panel.";
        }
        else
        {
            KLOG_INFO(xsettings) << "Restart panel successfully";
        }

        // 重置桌面图标大小
        if (m_backgroundSettings &&
            m_backgroundSettings->get(BACKGROUND_SCHEMA_SHOW_DESKTOP_ICONS).toBool() &&
            !m_showDesktopIconTimer->isActive())
        {
            KLOG_INFO(xsettings) << "Disable show-desktop-icon properties then enable to repaint the desktop icon.";
            m_backgroundSettings->set(BACKGROUND_SCHEMA_SHOW_DESKTOP_ICONS, false);
            // 延时显示桌面图标，给文件管理器一定的时间重绘
            m_showDesktopIconTimer->start(1000);
        }
    }
}

void XSettingsManager::processScreenChanged()
{
    auto scale = getWindowScale();
    if (scale != m_windowScale)
    {
        scaleSettings();
    }
}

void XSettingsManager::enableShowDesktopIcon()
{
    if (m_backgroundSettings)
    {
        m_backgroundSettings->set(BACKGROUND_SCHEMA_SHOW_DESKTOP_ICONS, true);
    }
    m_showDesktopIconTimer->stop();
}

void XSettingsManager::processFontconfigTimestampChanged()
{
    int32_t timestamp = time(NULL);
    m_registry->update(XSETTINGS_REGISTRY_PROP_FONTCONFIG_TIMESTAMP, timestamp);
}

void XSettingsManager::processPropertiesChanged(const QStringList &properties)
{
    Q_EMIT PropertiesChanged(properties);
}

}  // namespace Kiran