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

#include "keyboard-manager.h"
#include <QGSettings>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QX11Info>
#include "iso-translation.h"
#include "keyboard-i.h"
#include "keyboardadaptor.h"
#include "lib/base/base.h"
#include "xkb-help.h"
#include "xkb-rules-parser.h"

#include <X11/XKBlib.h>
#include <X11/Xlib.h>

namespace Kiran
{
#define KEYBOARD_SCHEMA_ID "com.kylinsec.kiran.keyboard"
#define KEYBOARD_SCHEMA_MODIFIER_LOCK_ENABLED "modifierLockEnabled"
#define KEYBOARD_SCHEMA_CAPSLOCK_TIPS_ENABLED "capslockTipsEnabled"
#define KEYBOARD_SCHEMA_NUMLOCK_TIPS_ENABLED "numlockTipsEnabled"
#define KEYBOARD_SCHEMA_REPEAT_ENABLED "repeatEnabled"
#define KEYBOARD_SCHEMA_REPEAT_DELAY "repeatDelay"
#define KEYBOARD_SCHEMA_REPEAT_INTERVAL "repeatInterval"
#define KEYBOARD_SCHEMA_LAYOUTS "layouts"
#define KEYBOARD_SCHEMA_OPTIONS "options"

#define LAYOUT_JOIN_CHAR ","
#define LAYOUT_MAX_NUMBER 4
#define DEFAULT_LAYOUT "us"
#define SETXKBMAP "setxkbmap"

#define DEFAULT_XKB_RULES_FILE KCC_XKB_BASE "/rules/base.xml"
#define DEFAULT_DESC_DELIMETERS " (,)"

KeyboardManager::KeyboardManager() : m_modifierLockEnabled(false),
                                     m_capslockTipsEnabled(false),
                                     m_numlockTipsEnabled(false),
                                     m_repeatEnabled(true),
                                     m_repeatDelay(500),
                                     m_repeatInterval(30)
{
    m_keyboardSettings = new QGSettings(KEYBOARD_SCHEMA_ID, "", this);
    m_keyboardAdaptor = new KeyboardAdaptor(this);
}

KeyboardManager::~KeyboardManager()
{
}

KeyboardManager *KeyboardManager::m_instance = nullptr;
void KeyboardManager::globalInit()
{
    m_instance = new KeyboardManager();
    m_instance->init();
}

#define SEND_PROPERTY_NOTIFY(property, propertyHump)                          \
    QVariantMap changedProperties;                                            \
    changedProperties.insert(QStringLiteral(#property), get##propertyHump()); \
                                                                              \
    QDBusMessage signalMessage = QDBusMessage::createSignal(                  \
        KEYBOARD_OBJECT_PATH,                                                 \
        QStringLiteral("org.freedesktop.DBus.Properties"),                    \
        QStringLiteral("PropertiesChanged"));                                 \
                                                                              \
    signalMessage.setArguments({                                              \
        KEYBOARD_DBUS_INTERFACE_NAME,                                         \
        changedProperties,                                                    \
        QStringList(),                                                        \
    });                                                                       \
    QDBusConnection::sessionBus().send(signalMessage);

void KeyboardManager::setCapslockTipsEnabled(bool capslockTipsEnabled)
{
    RETURN_IF_TRUE(capslockTipsEnabled == m_capslockTipsEnabled);

    m_capslockTipsEnabled = capslockTipsEnabled;
    m_keyboardSettings->set(KEYBOARD_SCHEMA_CAPSLOCK_TIPS_ENABLED, capslockTipsEnabled);
    SEND_PROPERTY_NOTIFY(capslock_tips_enabled, CapslockTipsEnabled);
}

void KeyboardManager::setLayouts(const QStringList &layouts)
{
    auto newLayouts = layouts;

    if (newLayouts.size() > LAYOUT_MAX_NUMBER)
    {
        KLOG_WARNING(keyboard) << "The number of the set layouts has" << newLayouts.size()
                               << ". It exceed max layout number" << LAYOUT_MAX_NUMBER << ". the subsequent layout is ignored.";
    }

    if (newLayouts.size() == 0)
    {
        KLOG_WARNING(keyboard) << "Because the user layout list is empty, so set the default layout 'us'.";
        newLayouts.push_back(DEFAULT_LAYOUT);
    }

    RETURN_IF_TRUE(m_layouts == newLayouts);

    if (setLayoutsToXkb(newLayouts))
    {
        m_layouts = newLayouts;
        m_keyboardSettings->set(KEYBOARD_SCHEMA_LAYOUTS, m_layouts);

        SEND_PROPERTY_NOTIFY(layouts, Layouts)
    }
}
void KeyboardManager::setModifierLockEnabled(bool modifierLockEnabled)
{
    RETURN_IF_TRUE(modifierLockEnabled == m_modifierLockEnabled);

    m_modifierLockEnabled = modifierLockEnabled;
    m_keyboardSettings->set(KEYBOARD_SCHEMA_MODIFIER_LOCK_ENABLED, modifierLockEnabled);
    SEND_PROPERTY_NOTIFY(modifier_lock_enabled, ModifierLockEnabled);
}

void KeyboardManager::setNumlockTipsEnabled(bool numlockTipsEnabled)
{
    RETURN_IF_TRUE(numlockTipsEnabled == m_numlockTipsEnabled);

    m_numlockTipsEnabled = numlockTipsEnabled;
    m_keyboardSettings->set(KEYBOARD_SCHEMA_NUMLOCK_TIPS_ENABLED, numlockTipsEnabled);
    SEND_PROPERTY_NOTIFY(numlock_tips_enabled, NumlockTipsEnabled);
}

void KeyboardManager::setOptions(const QStringList &options)
{
    RETURN_IF_TRUE(m_options == options);

    if (setOptionsToXkb(options))
    {
        m_options = options;
        m_keyboardSettings->set(KEYBOARD_SCHEMA_OPTIONS, m_options);
        SEND_PROPERTY_NOTIFY(options, Options)
    }
}
void KeyboardManager::setRepeatDelay(int repeatDelay)
{
    RETURN_IF_TRUE(m_repeatDelay == repeatDelay);

    m_repeatDelay = repeatDelay;
    m_keyboardSettings->set(KEYBOARD_SCHEMA_REPEAT_DELAY, repeatDelay);
    setAutoRepeatToXkb();

    SEND_PROPERTY_NOTIFY(repeat_delay, RepeatDelay);
}

void KeyboardManager::setRepeatEnabled(bool repeatEnabled)
{
    RETURN_IF_TRUE(m_repeatEnabled == repeatEnabled);

    m_repeatEnabled = repeatEnabled;
    m_keyboardSettings->set(KEYBOARD_SCHEMA_REPEAT_ENABLED, repeatEnabled);
    setAutoRepeatToXkb();

    SEND_PROPERTY_NOTIFY(repeat_enabled, RepeatEnabled);
}

void KeyboardManager::setRepeatInterval(int repeatInterval)
{
    RETURN_IF_TRUE(m_repeatInterval == repeatInterval);

    m_repeatInterval = repeatInterval;
    m_keyboardSettings->set(KEYBOARD_SCHEMA_REPEAT_INTERVAL, repeatInterval);
    setAutoRepeatToXkb();

    SEND_PROPERTY_NOTIFY(repeat_interval, RepeatInterval);
}

void KeyboardManager::AddLayout(const QString &layout)
{
    auto layouts = getLayouts();
    if (layouts.size() >= LAYOUT_MAX_NUMBER)
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_KEYBOARD_LAYOUT_EXCEED_LIMIT, LAYOUT_MAX_NUMBER);
    }

    if (m_validLayouts.find(layout) == m_validLayouts.end())
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_KEYBOARD_LAYOUT_INVALID);
    }

    if (layouts.contains(layout))
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_KEYBOARD_LAYOUT_ALREADY_EXIST);
    }

    layouts.push_back(layout);
    setLayouts(layouts);
}

void KeyboardManager::AddLayoutOption(const QString &option)
{
    auto options = m_options;

    if (m_options.contains(option))
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_KEYBOARD_LAYOUT_OPTION_ALREADY_EXIST);
    }

    options.push_back(option);
    setOptions(options);
}

void KeyboardManager::ApplyLayout(const QString &layout)
{
    auto layouts = getLayouts();

    if (!layouts.contains(layout))
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_KEYBOARD_LAYOUT_NOT_EXIST);
    }

    layouts.removeOne(layout);
    layouts.push_front(layout);
    setLayouts(layouts);
}

void KeyboardManager::ClearLayoutOption()
{
    setOptions(QStringList());
}

void KeyboardManager::DelLayout(const QString &layout)
{
    auto layouts = getLayouts();

    if (!layouts.contains(layout))
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_KEYBOARD_LAYOUT_NOT_EXIST);
    }
    layouts.removeOne(layout);
    setLayouts(layouts);
}

void KeyboardManager::DelLayoutOption(const QString &option)
{
    auto options = m_options;

    if (!m_options.contains(option))
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_KEYBOARD_LAYOUT_OPTION_NOT_EXIST);
    }
    options.removeOne(option);
    setOptions(options);
}
QString KeyboardManager::GetValidLayouts()
{
    QJsonDocument jsonDoc;
    QJsonArray jsonLayouts;

    for (auto &layoutName : m_validLayouts.keys())
    {
        QJsonObject jsonLayout;
        jsonLayout[KEYBOARD_VALID_LAYOUTS_LAYOUT_NAME] = layoutName;
        jsonLayout[KEYBOARD_VALID_LAYOUTS_COUNTRY_NAME] = m_validLayouts[layoutName];
        jsonLayouts.append(jsonLayout);
    }

    return QJsonDocument(jsonLayouts).toJson(QJsonDocument::Compact);
}

void KeyboardManager::SwitchCapsLockTips(bool enabled)
{
    setCapslockTipsEnabled(enabled);
}

void KeyboardManager::SwitchNumLockTips(bool enabled)
{
    setNumlockTipsEnabled(enabled);
}

#define AUTO_REPEAT_SET_HANDLER(prop, type1, key, type2)                                                           \
    bool KeyboardManager::prop##_setHandler(type1 value)                                                           \
    {                                                                                                              \
        RETURN_VAL_IF_TRUE(value == prop##_, false);                                                               \
        if (keyboard_settings_->get_##type2(key) != value)                                                         \
        {                                                                                                          \
            auto value_r = Glib::Variant<std::remove_cv<std::remove_reference<type1>::type>::type>::create(value); \
            if (!keyboard_settings_->set_value(key, value_r))                                                      \
            {                                                                                                      \
                return false;                                                                                      \
            }                                                                                                      \
        }                                                                                                          \
        prop##_ = value;                                                                                           \
        setAutoRepeatToXkb();                                                                                      \
        return true;                                                                                               \
    }

void KeyboardManager::init()
{
    int xkbEventBase = -1;

    if (!XkbHelp::xkbSupported(xkbEventBase))
    {
        return;
    }

    loadFromSettings();
    loadXkbRules();
    setAllPropsToXkb();

    connect(m_keyboardSettings, &QGSettings::changed, this, &KeyboardManager::processSettingsChanged);

    auto sessionConnection = QDBusConnection::sessionBus();
    if (!sessionConnection.registerService(KEYBOARD_DBUS_NAME))
    {
        KLOG_WARNING(audio) << "Failed to register dbus name:" << KEYBOARD_DBUS_NAME;
        return;
    }

    if (!sessionConnection.registerObject(KEYBOARD_OBJECT_PATH, KEYBOARD_DBUS_INTERFACE_NAME, this))
    {
        KLOG_ERROR(audio) << "Can't register object:" << sessionConnection.lastError();
        return;
    }
}

void KeyboardManager::loadFromSettings()
{
    m_modifierLockEnabled = m_keyboardSettings->get(KEYBOARD_SCHEMA_MODIFIER_LOCK_ENABLED).toBool();
    m_capslockTipsEnabled = m_keyboardSettings->get(KEYBOARD_SCHEMA_CAPSLOCK_TIPS_ENABLED).toBool();
    m_numlockTipsEnabled = m_keyboardSettings->get(KEYBOARD_SCHEMA_NUMLOCK_TIPS_ENABLED).toBool();
    m_repeatEnabled = m_keyboardSettings->get(KEYBOARD_SCHEMA_REPEAT_ENABLED).toBool();
    m_repeatDelay = m_keyboardSettings->get(KEYBOARD_SCHEMA_REPEAT_DELAY).toInt();
    m_repeatInterval = m_keyboardSettings->get(KEYBOARD_SCHEMA_REPEAT_INTERVAL).toInt();
    m_layouts = m_keyboardSettings->get(KEYBOARD_SCHEMA_LAYOUTS).toStringList();
    m_options = m_keyboardSettings->get(KEYBOARD_SCHEMA_OPTIONS).toStringList();
}

void KeyboardManager::processSettingsChanged(const QString &key)
{
    switch (shash(key.toLatin1().data()))
    {
    case CONNECT(KEYBOARD_SCHEMA_REPEAT_ENABLED, _hash):
        setRepeatEnabled(m_keyboardSettings->get(key).toBool());
        break;
    case CONNECT(KEYBOARD_SCHEMA_REPEAT_DELAY, _hash):
        setRepeatDelay(m_keyboardSettings->get(key).toInt());
        break;
    case CONNECT(KEYBOARD_SCHEMA_REPEAT_INTERVAL, _hash):
        setRepeatInterval(m_keyboardSettings->get(key).toInt());
        break;
    case CONNECT(KEYBOARD_SCHEMA_LAYOUTS, _hash):
        setLayouts(m_keyboardSettings->get(key).toStringList());
        break;
    case CONNECT(KEYBOARD_SCHEMA_OPTIONS, _hash):
        setOptions(m_keyboardSettings->get(key).toStringList());
        break;
    case CONNECT(KEYBOARD_SCHEMA_CAPSLOCK_TIPS_ENABLED, _hash):
        setCapslockTipsEnabled(m_keyboardSettings->get(key).toBool());
        break;
    case CONNECT(KEYBOARD_SCHEMA_NUMLOCK_TIPS_ENABLED, _hash):
        setNumlockTipsEnabled(m_keyboardSettings->get(key).toBool());
        break;
    default:
        break;
    }
}

void KeyboardManager::loadXkbRules()
{
    XkbRulesParser rulesParser(DEFAULT_XKB_RULES_FILE);
    XkbRules xkbRules;
    QString error;
    if (!rulesParser.parse(xkbRules, error))
    {
        KLOG_WARNING(keyboard) << "Failed to parse file" << DEFAULT_XKB_RULES_FILE << ", error is" << error;
        return;
    }

    for (int i = 0; i < xkbRules.layouts.size(); ++i)
    {
        auto &layoutName = xkbRules.layouts[i].name;
        auto countryName = ISOTranslation::getInstance()->getLocaleCountryName(layoutName.toUpper());
        if (countryName.length() > 0)
        {
            m_validLayouts[layoutName] = countryName;
        }
        else
        {
            m_validLayouts[layoutName] = ISOTranslation::getInstance()->getLocaleString(xkbRules.layouts[i].description, DEFAULT_DESC_DELIMETERS);
        }

        KLOG_DEBUG(keyboard) << "name is" << layoutName << ", value is" << m_validLayouts[layoutName];

        for (int j = 0; j < xkbRules.layouts[i].variants.size(); ++j)
        {
            auto layoutVariant = layoutName + " " + xkbRules.layouts[i].variants[j].name;
            auto variantDesc = ISOTranslation::getInstance()->getLocaleString(xkbRules.layouts[i].variants[j].description, DEFAULT_DESC_DELIMETERS);
            auto desciption = m_validLayouts[layoutName] + " " + variantDesc;
            m_validLayouts[layoutVariant] = desciption;

            KLOG_DEBUG(keyboard) << "name is" << layoutVariant << ", value is" << desciption;
        }
    }
}

void KeyboardManager::setAllPropsToXkb()
{
    setAutoRepeatToXkb();
    setLayoutsToXkb(m_layouts);
    setOptionsToXkb(m_options);
}

void KeyboardManager::setAutoRepeatToXkb()
{
    KLOG_INFO(keyboard) << "Set auto repeat to xbk."
                        << "enabled is" << m_repeatEnabled
                        << ", repeat delay is" << m_repeatDelay
                        << ", repeat interval is" << m_repeatInterval;

    auto display = QX11Info::display();

    if (m_repeatEnabled)
    {
        XAutoRepeatOn(display);

        auto ret = XkbSetAutoRepeatRate(display,
                                        XkbUseCoreKbd,
                                        m_repeatDelay,
                                        m_repeatInterval);

        if (!ret)
        {
            KLOG_WARNING(keyboard) << "XKeyboard keyboard extensions are unavailable, no way to support keyboard autorepeat rate settings";
        }
    }
    else
    {
        XAutoRepeatOff(display);
    }

    XFlush(display);
}

bool KeyboardManager::setLayoutsToXkb(const QStringList &layouts)
{
    QString joinLayouts;
    QString joinVariants;
    for (auto layout : layouts)
    {
        auto layoutVariant = layout.split(' ', Qt::SkipEmptyParts);

        if (layoutVariant.size() == 1)
        {
            joinLayouts += (layoutVariant[0] + LAYOUT_JOIN_CHAR);
            joinVariants += LAYOUT_JOIN_CHAR;
        }
        else if (layoutVariant.size() == 2)
        {
            joinLayouts += (layoutVariant[0] + LAYOUT_JOIN_CHAR);
            joinVariants += (layoutVariant[1] + LAYOUT_JOIN_CHAR);
        }
        else
        {
            KLOG_WARNING(keyboard) << "The format of the layout item" << layout << "is invalid. it's already ignored";
        }
    }

    if (joinLayouts.length() == 0)
    {
        joinLayouts = DEFAULT_LAYOUT LAYOUT_JOIN_CHAR;
        joinVariants = QString(LAYOUT_JOIN_CHAR);
    }

    auto arguments = QStringList{"-layout", joinLayouts, "-variant", joinVariants};

    KLOG_INFO(keyboard) << "Set layouts to xkb by running command" << QString("%1 %2").arg(SETXKBMAP).arg(arguments.join(' '));
    auto exitCode = QProcess::execute(SETXKBMAP, arguments);
    if (exitCode != 0)
    {
        KLOG_WARNING(keyboard) << "Failed to set layouts, exit code is" << exitCode;
        return false;
    }

    return true;
}

bool KeyboardManager::setOptionsToXkb(const QStringList &options)
{
    auto exitCode = QProcess::execute(SETXKBMAP, QStringList{"-option"});
    if (exitCode != 0)
    {
        KLOG_WARNING(keyboard) << "Execute command 'setxkbmap -option', exit code is" << exitCode;
        return false;
    }

    QStringList arguments;
    for (auto option : options)
    {
        arguments.append("-option");
        arguments.append(option);
    }

    RETURN_VAL_IF_TRUE(arguments.length() == 0, true);

    KLOG_INFO(keyboard) << "Set options to xkb by running command" << QString("%1 %2").arg(SETXKBMAP).arg(arguments.join(' '));
    exitCode = QProcess::execute(SETXKBMAP, arguments);
    if (exitCode != 0)
    {
        KLOG_WARNING(keyboard) << "Failed to set options, exit code is" << exitCode;
        return false;
    }

    return true;
}

}  // namespace Kiran