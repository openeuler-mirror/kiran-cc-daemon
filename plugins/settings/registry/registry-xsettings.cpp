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

#include "registry-xsettings.h"
#include <xcb/xcb.h>
#include <QGSettings>
#include <QTimer>
#include "../settings-manager.h"
#include "../settings-utils.h"
#include "fontconfig-monitor.h"
#include "lib/base/base.h"
#include "lib/xcb/xcb-connection.h"
#include "settings-i.h"

namespace Kiran
{
#define XSETTINGS_PAD(n, m) ((n + m - 1) & (~(m - 1)))

const QMap<QString, QString> RegistryXSettings::m_schema2Registry =
    {
        {SETTINGS_SCHEMA_NET_DOUBLE_CLICK_TIME, XSETTINGS_REGISTRY_PROP_NET_DOUBLE_CLICK_TIME},
        {SETTINGS_SCHEMA_NET_DOUBLE_CLICK_DISTANCE, XSETTINGS_REGISTRY_PROP_NET_DOUBLE_CLICK_DISTANCE},
        {SETTINGS_SCHEMA_NET_DND_DRAG_THRESHOLD, XSETTINGS_REGISTRY_PROP_NET_DND_DRAG_THRESHOLD},
        {SETTINGS_SCHEMA_NET_CURSOR_BLINK, XSETTINGS_REGISTRY_PROP_NET_CURSOR_BLINK},
        {SETTINGS_SCHEMA_NET_CURSOR_BLINK_TIME, XSETTINGS_REGISTRY_PROP_NET_CURSOR_BLINK_TIME},
        {SETTINGS_SCHEMA_NET_THEME_NAME, XSETTINGS_REGISTRY_PROP_NET_THEME_NAME},
        {SETTINGS_SCHEMA_NET_ICON_THEME_NAME, XSETTINGS_REGISTRY_PROP_NET_ICON_THEME_NAME},
        {SETTINGS_SCHEMA_NET_ENABLE_EVENT_SOUNDS, XSETTINGS_REGISTRY_PROP_NET_ENABLE_EVENT_SOUNDS},
        {SETTINGS_SCHEMA_NET_SOUND_THEME_NAME, XSETTINGS_REGISTRY_PROP_NET_SOUND_THEME_NAME},
        {SETTINGS_SCHEMA_NET_ENABLE_INPUT_FEEDBACK_SOUNDS, XSETTINGS_REGISTRY_PROP_NET_ENABLE_INPUT_FEEDBACK_SOUNDS},

        {SETTINGS_SCHEMA_XFT_ANTIALIAS, XSETTINGS_REGISTRY_PROP_XFT_ANTIALIAS},
        {SETTINGS_SCHEMA_XFT_HINTING, XSETTINGS_REGISTRY_PROP_XFT_HINTING},
        {SETTINGS_SCHEMA_XFT_HINT_STYLE, XSETTINGS_REGISTRY_PROP_XFT_HINT_STYLE},
        {SETTINGS_SCHEMA_XFT_RGBA, XSETTINGS_REGISTRY_PROP_XFT_RGBA},
        {SETTINGS_SCHEMA_XFT_DPI, XSETTINGS_REGISTRY_PROP_XFT_DPI},

        {SETTINGS_SCHEMA_GTK_CURSOR_THEME_NAME, XSETTINGS_REGISTRY_PROP_GTK_CURSOR_THEME_NAME},
        {SETTINGS_SCHEMA_GTK_CURSOR_THEME_SIZE, XSETTINGS_REGISTRY_PROP_GTK_CURSOR_THEME_SIZE},
        {SETTINGS_SCHEMA_GTK_FONT_NAME, XSETTINGS_REGISTRY_PROP_GTK_FONT_NAME},
        {SETTINGS_SCHEMA_GTK_KEY_THEME_NAME, XSETTINGS_REGISTRY_PROP_GTK_KEY_THEME_NAME},
        {SETTINGS_SCHEMA_GTK_TOOLBAR_STYLE, XSETTINGS_REGISTRY_PROP_GTK_TOOLBAR_STYLE},
        {SETTINGS_SCHEMA_GTK_TOOLBAR_ICONS_SIZE, XSETTINGS_REGISTRY_PROP_GTK_TOOLBAR_ICON_SIZE},
        {SETTINGS_SCHEMA_GTK_IM_PREEDIT_STYLE, XSETTINGS_REGISTRY_PROP_GTK_IM_PREEDIT_STYLE},
        {SETTINGS_SCHEMA_GTK_IM_STATUS_STYLE, XSETTINGS_REGISTRY_PROP_GTK_IM_STATUS_STYLE},
        {SETTINGS_SCHEMA_GTK_IM_MODULE, XSETTINGS_REGISTRY_PROP_GTK_IM_MODULE},
        {SETTINGS_SCHEMA_GTK_MENU_IMAGES, XSETTINGS_REGISTRY_PROP_GTK_MENU_IMAGES},
        {SETTINGS_SCHEMA_GTK_BUTTON_IMAGES, XSETTINGS_REGISTRY_PROP_GTK_BUTTON_IMAGES},
        {SETTINGS_SCHEMA_GTK_MENUBAR_ACCEL, XSETTINGS_REGISTRY_PROP_GTK_MENU_BAR_ACCEL},
        {SETTINGS_SCHEMA_GTK_COLOR_SCHEME, XSETTINGS_REGISTRY_PROP_GTK_COLOR_SCHEME},
        {SETTINGS_SCHEMA_GTK_FILE_CHOOSER_BACKEND, XSETTINGS_REGISTRY_PROP_GTK_FILE_CHOOSER_BACKEND},
        {SETTINGS_SCHEMA_GTK_DECORATION_LAYOUT, XSETTINGS_REGISTRY_PROP_GTK_DECORATION_LAYOUT},
        {SETTINGS_SCHEMA_GTK_SHELL_SHOWS_APP_MENU, XSETTINGS_REGISTRY_PROP_GTK_SHELL_SHOWS_APP_MENU},
        {SETTINGS_SCHEMA_GTK_SHELL_SHOWS_MENUBAR, XSETTINGS_REGISTRY_PROP_GTK_SHELL_SHOWS_MENUBAR},
        {SETTINGS_SCHEMA_GTK_SHOW_INPUT_METHOD_MENU, XSETTINGS_REGISTRY_PROP_GTK_SHOW_INPUT_METHOD_MENU},
        {SETTINGS_SCHEMA_GTK_SHOW_UNICODE_MENU, XSETTINGS_REGISTRY_PROP_GTK_SHOW_UNICODE_MENU},
        {SETTINGS_SCHEMA_GTK_AUTOMATIC_MNEMONICS, XSETTINGS_REGISTRY_PROP_GTK_AUTO_MNEMONICS},
        {SETTINGS_SCHEMA_GTK_ENABLE_PRIMARY_PASTE, XSETTINGS_REGISTRY_PROP_GTK_ENABLE_PRIMARY_PASTE},
        {SETTINGS_SCHEMA_GTK_ENABLE_ANIMATIONS, XSETTINGS_REGISTRY_PROP_GTK_ENABLE_ANIMATIONS},
        {SETTINGS_SCHEMA_GTK_DIALOGS_USE_HEADER, XSETTINGS_REGISTRY_PROP_GTK_DIALOGS_USE_HEADER},

        {SETTINGS_SCHEMA_WINDOW_SCALING_FACTOR, XSETTINGS_REGISTRY_PROP_GDK_WINDOW_SCALING_FACTOR},
};

XSettingsPropertyBase::XSettingsPropertyBase(const QString &name,
                                             XSettingsPropType type,
                                             uint32_t serial) : m_name(name),
                                                                m_type(type),
                                                                m_lastChangeSerial(serial)
{
}

QByteArray XSettingsPropertyBase::serialize()
{
    QByteArray data;
    data.push_back(char(m_type));
    data.push_back(char('\0'));

    // 类型不能随意修改
    uint16_t nameLen = m_name.length();
    uint16_t nameLenPad = XSETTINGS_PAD(nameLen, 4) - nameLen;
    data.append(QByteArray((const char *)&nameLen, 2));
    data.append(m_name.toUtf8());
    data.append(QByteArray(nameLenPad, '\0'));
    data.append(QByteArray((const char *)&m_lastChangeSerial, 4));
    return data;
}

XSettingsPropertyInt::XSettingsPropertyInt(const QString &name,
                                           int32_t value,
                                           uint32_t serial) : XSettingsPropertyBase(name, XSettingsPropType::XSETTINGS_PROP_TYPE_INT, serial),
                                                              m_value(value)
{
}

bool XSettingsPropertyInt::operator==(const XSettingsPropertyBase &rval) const
{
    if (rval.getType() != XSettingsPropType::XSETTINGS_PROP_TYPE_INT)
    {
        KLOG_WARNING(settings) << "Unsupported.";
        return false;
    }
    return operator==(dynamic_cast<const XSettingsPropertyInt &>(rval));
}

bool XSettingsPropertyInt::operator==(const XSettingsPropertyInt &rval) const
{
    return (getName() == rval.getName() && m_value == rval.m_value);
}

QByteArray XSettingsPropertyInt::serialize()
{
    QByteArray data;
    data = XSettingsPropertyBase::serialize();
    data.append(QByteArray((const char *)&m_value, 4));
    return data;
}

XSettingsPropertyString::XSettingsPropertyString(const QString &name,
                                                 const QString &value,
                                                 uint32_t serial) : XSettingsPropertyBase(name, XSettingsPropType::XSETTINGS_PROP_TYPE_STRING, serial),
                                                                    m_value(value)
{
}

bool XSettingsPropertyString::operator==(const XSettingsPropertyBase &rval) const
{
    if (rval.getType() != XSettingsPropType::XSETTINGS_PROP_TYPE_STRING)
    {
        KLOG_WARNING(settings) << "Unsupported.";
        return false;
    }
    return operator==(dynamic_cast<const XSettingsPropertyString &>(rval));
}

bool XSettingsPropertyString::operator==(const XSettingsPropertyString &rval) const
{
    return (getName() == rval.getName() && m_value == rval.m_value);
}

QByteArray XSettingsPropertyString::serialize()
{
    QByteArray data;
    data = XSettingsPropertyBase::serialize();
    uint32_t strLen = m_value.length();
    uint32_t strLenPad = XSETTINGS_PAD(strLen, 4) - strLen;
    data.append(QByteArray((const char *)&strLen, 4));
    data.append(m_value.toUtf8());
    data.append(QByteArray(strLenPad, '\0'));
    return data;
}

XSettingsPropertyColor::XSettingsPropertyColor(const QString &name,
                                               const XSettingsColor &value,
                                               uint32_t serial) : XSettingsPropertyBase(name, XSettingsPropType::XSETTINGS_PROP_TYPE_COLOR, serial),
                                                                  m_value(value)
{
}

bool XSettingsPropertyColor::operator==(const XSettingsPropertyBase &rval) const
{
    if (rval.getType() != XSettingsPropType::XSETTINGS_PROP_TYPE_COLOR)
    {
        KLOG_WARNING(settings) << "Unsupported.";
        return false;
    }
    return operator==(dynamic_cast<const XSettingsPropertyColor &>(rval));
}

bool XSettingsPropertyColor::operator==(const XSettingsPropertyColor &rval) const
{
    return (getName() == rval.getName() && m_value == rval.m_value);
}

QByteArray XSettingsPropertyColor::serialize()
{
    QByteArray data;
    data = XSettingsPropertyBase::serialize();
    data.append(QByteArray((const char *)&m_value.red, 2));
    data.append(QByteArray((const char *)&m_value.green, 2));
    data.append(QByteArray((const char *)&m_value.blue, 2));
    data.append(QByteArray((const char *)&m_value.alpha, 2));
    return data;
}

RegistryXSettings::RegistryXSettings(SettingsManager *manager) : QObject(manager),
                                                                 m_manager(manager),
                                                                 m_serial(0)
{
    m_xcbConnection = XcbConnection::getDefault();

    auto name = QString("_XSETTINGS_S%1").arg(m_xcbConnection->getDefaultScreenNumber());
    auto reply = XCB_REPLY(xcb_intern_atom, m_xcbConnection->getConnection(), false, name.size(), name.toUtf8().data());
    m_selectionAtom = reply ? reply->atom : XCB_ATOM_NONE;

    reply = XCB_REPLY(xcb_intern_atom, m_xcbConnection->getConnection(), false, strlen("_XSETTINGS_SETTINGS"), "_XSETTINGS_SETTINGS");
    m_xsettingsAtom = reply ? reply->atom : XCB_ATOM_NONE;

    reply = XCB_REPLY(xcb_intern_atom, m_xcbConnection->getConnection(), false, strlen("MANAGER"), "MANAGER");
    m_managerAtom = reply ? reply->atom : XCB_ATOM_NONE;

    m_timer = new QTimer(this);
    m_settings = new QGSettings(SETTINGS_SCHEMA_ID, "", this);
    m_fontconfigMonitor = new FontconfigMonitor(this);
}

RegistryXSettings::~RegistryXSettings()
{
}

void RegistryXSettings::init()
{
    // 检查是否有其他xsettings插件已经在运行

    auto xcbConnection = m_xcbConnection->getConnection();
    auto rootWindow = m_xcbConnection->getDefaultScreen()->root;

    auto reply = XCB_REPLY(xcb_get_selection_owner, xcbConnection, m_selectionAtom);
    if (reply && reply->owner != XCB_NONE)
    {
        KLOG_WARNING(settings) << "You can only run one xsettings manager at a time.";
        return;
    }

    m_xsettingsWindow = xcb_generate_id(xcbConnection);
    xcb_create_window(xcbConnection,
                      XCB_COPY_FROM_PARENT,  // depth -- same as root
                      m_xsettingsWindow,     // window id
                      rootWindow,            // root window id
                      0, 0, 10, 10,
                      0,                     // border width
                      XCB_COPY_FROM_PARENT,  // window class
                      XCB_COPY_FROM_PARENT,  // visual
                      0,                     // value mask
                      nullptr);              // value list

    auto name = QString("_XSETTINGS_S%1").arg(m_xcbConnection->getDefaultScreenNumber());
    xcb_set_selection_owner(xcbConnection, m_xsettingsWindow, m_selectionAtom, XCB_TIME_CURRENT_TIME);
    // 判断设置Owner是否成功
    auto getSelectionReply = XCB_REPLY(xcb_get_selection_owner, xcbConnection, m_selectionAtom);
    if (!getSelectionReply)
    {
        KLOG_WARNING(settings) << "Failed to get selection owner for" << name;
        return;
    }
    if (getSelectionReply->owner != m_xsettingsWindow)
    {
        KLOG_WARNING(settings) << "Failed to set selection owner" << m_xsettingsWindow
                               << "for" << name << "which not equal to" << getSelectionReply->owner;
        return;
    }

    KLOG_INFO(settings) << "Success to set owner selection" << name << "to" << m_xsettingsWindow
                        << ", parent window is" << rootWindow;

    // 其他客户端通过监听"MANAGER"事件来感知Xsettings Owner是否创建
    xcb_client_message_event_t event;
    event.response_type = XCB_CLIENT_MESSAGE;
    event.window = rootWindow;
    event.format = 32;
    event.type = m_managerAtom;
    event.data.data32[0] = XCB_TIME_CURRENT_TIME;
    event.data.data32[1] = m_selectionAtom;
    event.data.data32[2] = m_xsettingsWindow;
    event.data.data32[3] = 0;
    event.data.data32[4] = 0;
    xcb_send_event(xcbConnection, false, rootWindow, XCB_EVENT_MASK_STRUCTURE_NOTIFY, (char *)&event);

    for (const auto &key : m_settings->keys())
    {
        sync2XSettings(key);
    }

    m_fontconfigMonitor->init();

    connect(m_timer, &QTimer::timeout, this, &RegistryXSettings::notify);
    connect(m_settings, &QGSettings::changed, std::bind(&RegistryXSettings::sync2XSettings, this, std::placeholders::_1));
    connect(m_fontconfigMonitor, &FontconfigMonitor::timestampChanged, this, &RegistryXSettings::processFontconfigTimestampChanged);
}

bool RegistryXSettings::update(const QString &name, int32_t value)
{
    auto var = QSharedPointer<XSettingsPropertyInt>::create(name, value, m_serial);
    return update(var);
}

bool RegistryXSettings::update(const QString &name, const QString &value)
{
    auto var = QSharedPointer<XSettingsPropertyString>::create(name, value, m_serial);
    return update(var);
}

bool RegistryXSettings::update(const QString &name, const XSettingsColor &value)
{
    auto var = QSharedPointer<XSettingsPropertyColor>::create(name, value, m_serial);
    return update(var);
}

bool RegistryXSettings::update(QSharedPointer<XSettingsPropertyBase> var)
{
    RETURN_VAL_IF_TRUE(var == nullptr, true);
    auto oldVar = getProperty(var->getName());
    if (oldVar != nullptr && *oldVar == *var)
    {
        return true;
    }

    m_properties.remove(var->getName());
    m_properties.insert(var->getName(), var);

    // 空闲时修改，因为update可能会被连续调用多次。
    m_timer->start(100);
    return true;
}

void RegistryXSettings::sync2XSettings(const QString &key)
{
    auto iter = m_schema2Registry.find(key);

#define SET_CASE(prop, type)                          \
    case CONNECT(prop, _hash):                        \
    {                                                 \
        auto value = m_settings->get(key).to##type(); \
        this->update(iter.value(), value);            \
        break;                                        \
    }

    switch (shash(key.toLatin1().data()))
    {
        SET_CASE(SETTINGS_SCHEMA_NET_DOUBLE_CLICK_TIME, Int);
        SET_CASE(SETTINGS_SCHEMA_NET_DOUBLE_CLICK_DISTANCE, Int);
        SET_CASE(SETTINGS_SCHEMA_NET_DND_DRAG_THRESHOLD, Int);
        SET_CASE(SETTINGS_SCHEMA_NET_CURSOR_BLINK, Bool);
        SET_CASE(SETTINGS_SCHEMA_NET_CURSOR_BLINK_TIME, Int);
        SET_CASE(SETTINGS_SCHEMA_NET_THEME_NAME, String);
        SET_CASE(SETTINGS_SCHEMA_NET_ICON_THEME_NAME, String);
        SET_CASE(SETTINGS_SCHEMA_NET_ENABLE_EVENT_SOUNDS, Bool);
        SET_CASE(SETTINGS_SCHEMA_NET_SOUND_THEME_NAME, String);
        SET_CASE(SETTINGS_SCHEMA_NET_ENABLE_INPUT_FEEDBACK_SOUNDS, Bool);

        SET_CASE(SETTINGS_SCHEMA_XFT_ANTIALIAS, Int);
        SET_CASE(SETTINGS_SCHEMA_XFT_HINTING, Int);
        SET_CASE(SETTINGS_SCHEMA_XFT_HINT_STYLE, String);
        SET_CASE(SETTINGS_SCHEMA_XFT_RGBA, String);

        SET_CASE(SETTINGS_SCHEMA_GTK_CURSOR_THEME_NAME, String);
        SET_CASE(SETTINGS_SCHEMA_GTK_FONT_NAME, String);
        SET_CASE(SETTINGS_SCHEMA_GTK_KEY_THEME_NAME, String);
        SET_CASE(SETTINGS_SCHEMA_GTK_TOOLBAR_STYLE, String);
        SET_CASE(SETTINGS_SCHEMA_GTK_TOOLBAR_ICONS_SIZE, String);
        SET_CASE(SETTINGS_SCHEMA_GTK_IM_PREEDIT_STYLE, String);
        SET_CASE(SETTINGS_SCHEMA_GTK_IM_STATUS_STYLE, String);
        SET_CASE(SETTINGS_SCHEMA_GTK_IM_MODULE, String);
        SET_CASE(SETTINGS_SCHEMA_GTK_MENU_IMAGES, Bool);
        SET_CASE(SETTINGS_SCHEMA_GTK_BUTTON_IMAGES, Bool);
        SET_CASE(SETTINGS_SCHEMA_GTK_MENUBAR_ACCEL, String);
        SET_CASE(SETTINGS_SCHEMA_GTK_COLOR_SCHEME, String);
        SET_CASE(SETTINGS_SCHEMA_GTK_FILE_CHOOSER_BACKEND, String);
        SET_CASE(SETTINGS_SCHEMA_GTK_DECORATION_LAYOUT, String);
        SET_CASE(SETTINGS_SCHEMA_GTK_SHELL_SHOWS_APP_MENU, Bool);
        SET_CASE(SETTINGS_SCHEMA_GTK_SHELL_SHOWS_MENUBAR, Bool);
        SET_CASE(SETTINGS_SCHEMA_GTK_SHOW_INPUT_METHOD_MENU, Bool);
        SET_CASE(SETTINGS_SCHEMA_GTK_SHOW_UNICODE_MENU, Bool);
        SET_CASE(SETTINGS_SCHEMA_GTK_AUTOMATIC_MNEMONICS, Bool);
        SET_CASE(SETTINGS_SCHEMA_GTK_ENABLE_PRIMARY_PASTE, Bool);
        SET_CASE(SETTINGS_SCHEMA_GTK_ENABLE_ANIMATIONS, Bool);
        SET_CASE(SETTINGS_SCHEMA_GTK_DIALOGS_USE_HEADER, Bool);

        // Ignore these properties
    case CONNECT(SETTINGS_SCHEMA_GTK_CURSOR_THEME_SIZE, _hash):
    case CONNECT(SETTINGS_SCHEMA_WINDOW_SCALING_FACTOR, _hash):
    case CONNECT(SETTINGS_SCHEMA_WINDOW_SCALING_FACTOR_QT_SYNC, _hash):
    case CONNECT(SETTINGS_SCHEMA_RELOAD_WHEN_SCALING, _hash):
    case CONNECT(SETTINGS_SCHEMA_XFT_DPI, _hash):
    case CONNECT(SETTINGS_SCHEMA_FONT_DPI, _hash):
    case CONNECT(SETTINGS_SCHEMA_GTK_CURSOR_BLINK_TIMEOUT, _hash):
        break;

    default:
        KLOG_WARNING(settings) << "Unknown key" << key;
        break;
    }
#undef SET_CASET

    switch (shash(key.toLatin1().data()))
    {
    case CONNECT(SETTINGS_SCHEMA_WINDOW_SCALING_FACTOR, _hash):
    case CONNECT(SETTINGS_SCHEMA_GTK_CURSOR_THEME_SIZE, _hash):
    case CONNECT(SETTINGS_SCHEMA_FONT_DPI, _hash):
    {
        auto scale = m_manager->getWindowScale();
        auto dpi = m_manager->getOptimizeDPI();
        int unscaledDPI = int(dpi * 1024);
        int scaledDPI = int(SettingsUtils::formatScaleDPI(scale, dpi) * 1024);
        auto scaledCursorSize = m_manager->getCursorThemeSize() * scale;

        this->update(XSETTINGS_REGISTRY_PROP_GDK_WINDOW_SCALING_FACTOR, scale);
        this->update(XSETTINGS_REGISTRY_PROP_GDK_UNSCALED_DPI, unscaledDPI);
        this->update(XSETTINGS_REGISTRY_PROP_XFT_DPI, scaledDPI);
        this->update(XSETTINGS_REGISTRY_PROP_GTK_CURSOR_THEME_SIZE, scaledCursorSize);
    }
    break;
    case CONNECT(SETTINGS_SCHEMA_XFT_RGBA, _hash):
        this->update(XSETTINGS_REGISTRY_PROP_XFT_LCDFILTER, m_manager->getXftRGBA() == "rgb" ? "lcddefault" : "none");
        break;
    default:
        break;
    }

    this->update(XSETTINGS_REGISTRY_PROP_NET_FALLBACK_ICON_THEME, "mate");
}

void RegistryXSettings::processFontconfigTimestampChanged()
{
    int32_t timestamp = time(NULL);
    this->update(XSETTINGS_REGISTRY_PROP_FONTCONFIG_TIMESTAMP, timestamp);
}

void RegistryXSettings::notify()
{
    QByteArray data;

    m_timer->stop();

    // 注意：填充的相关变量类型不能随意修改
    // 填充head：byte-order + pad + SERIAL + N_SETTINGS
    int32_t nsettings = m_properties.size();
    data.push_back(byteOrder());
    data.append(QByteArray("\0\0\0", 3));
    data.append(QByteArray((const char *)&m_serial, 4));
    data.append(QByteArray((const char *)&nsettings, 4));
    ++m_serial;

    // 填充body
    for (const auto &property : m_properties)
    {
        data.append(property->serialize());
    }

    KLOG_INFO(settings) << "Changed _XSETTINGS_SETTINGS properties to" << data;

    xcb_change_property(m_xcbConnection->getConnection(),
                        XCB_PROP_MODE_REPLACE,
                        m_xsettingsWindow,
                        m_xsettingsAtom,
                        m_xsettingsAtom,
                        8,
                        data.length(),
                        data.data());

    /* 这里必须要刷新，否则数据可能在缓存中导致未立即生效，引发一些问题，例如设置QT光标不生效：
    1. 通过控制中心前端设置光标主题
    2. 控制中心后端收到设置光标主题dbus请求，修改gsettings的gtk-cursor-theme-name
        2.1 启动定制器(100ms延时），定时器触发后控制中心后端执行当前函数，更新_XSETTINGS_SETTINGS
        2.2 kiran QPA主题插件启动定时器（500ms延时），定时器触发后告知QT刷新光标主题，QT从_XSETTINGS_SETTINGS中获取数据进行刷新
    如果2.1步中更新_XSETTINGS_SETTINGS到xserver没有立即生效（在缓存中），就会导致QT获取到的是旧的数据，导致光标主题设置失败，
    就算后续数据更新了，QT也不会再执行刷新动作。*/
    xcb_flush(m_xcbConnection->getConnection());
}

char RegistryXSettings::byteOrder()
{
    // sonarqube block off
    // 在指定机器上，下面的三元运算符永远为true，所以sonarqube会报code smell，这是正常的
    uint32_t myint = 0x01020304;
    return (*reinterpret_cast<char *>(&myint) == 1) ? XCB_IMAGE_ORDER_MSB_FIRST : XCB_IMAGE_ORDER_LSB_FIRST;
    // sonarqube block on
}

}  // namespace Kiran
