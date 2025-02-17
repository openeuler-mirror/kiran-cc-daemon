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
 * Author:     meizhigang <meizhigang@kylinsec.com.cn>
 */

#include "modifier-lock-manager.h"
#include <QCoreApplication>
#include "lib/base/base.h"
#include "lib/osdwindow/osd-window.h"

#include <X11/XKBlib.h>

#define XK_MISCELLANY
#include <X11/keysymdef.h>
#undef XK_MISCELLANY

#define explicit explicit_is_keyword_in_cpp
#include <xcb/xkb.h>
#undef explicit

#include <QX11Info>
#include "keyboard-manager.h"
#include "xkb-help.h"

namespace Kiran
{
#define IMAGE_CAPSLOCK_ENABLED "osd-capslock-enabled"
#define IMAGE_CAPSLOCK_DISABLED "osd-capslock-disabled"
#define IMAGE_NUMLOCK_ENABLED "osd-numlock-enabled"
#define IMAGE_NUMLOCK_DISABLED "osd-numlock-disabled"

typedef union
{
    struct
    {
        uint8_t responseType;
        uint8_t xkbType;
        uint16_t sequence;
        xcb_timestamp_t time;
        uint8_t deviceID;
    } any;
    xcb_xkb_state_notify_event_t stateNotify;
} XkbEvent;

ModifierLockManager::ModifierLockManager(KeyboardManager *keyboardManager) : m_keyboardManager(keyboardManager)
{
}

ModifierLockManager::~ModifierLockManager()
{
}

ModifierLockManager *ModifierLockManager::m_instance = nullptr;

void ModifierLockManager::globalInit(KeyboardManager *keyboardManager)
{
    m_instance = new ModifierLockManager(keyboardManager);

    if (keyboardManager->getModifierLockEnabled())
    {
        m_instance->init();
    }
}

void ModifierLockManager::init()
{
    auto dpy = QX11Info::display();

    m_capslockMask = XkbKeysymToModifiers(dpy, XK_Caps_Lock);
    m_numlockMask = XkbKeysymToModifiers(dpy, XK_Num_Lock);
    m_capslockKeycode = XKeysymToKeycode(dpy, XK_Caps_Lock);
    m_numlockKeycode = XKeysymToKeycode(dpy, XK_Num_Lock);

    KLOG_DEBUG(keyboard) << "Init xkb keysym information. capslock mask is" << m_capslockMask
                         << ", keycode is" << m_capslockKeycode
                         << ", numlock mask is" << m_numlockMask
                         << ", keycode is" << m_numlockKeycode;

    if (QCoreApplication::instance() != nullptr && XkbHelp::xkbSupported(m_xkbEventBase))
    {
        int eventMask = XkbNewKeyboardNotifyMask | XkbStateNotifyMask;
        if (!XkbSelectEvents(dpy, XkbUseCoreKbd, eventMask, eventMask))
        {
            KLOG_WARNING(keyboard) << "Couldn't select desired XKB events";
            return;
        }
        QCoreApplication::instance()->installNativeEventFilter(this);
    }
}

void ModifierLockManager::setLockAction(xcb_keycode_t keycode, uint8_t mods)
{
    if (keycode == m_capslockKeycode)
    {
        RETURN_IF_FALSE(m_keyboardManager->getCapslockTipsEnabled());

        bool capslockEnable = !!(m_capslockMask & mods);
        if (capslockEnable)
        {
            OSDWindow::getInstance()->showIcon(IMAGE_CAPSLOCK_ENABLED);
        }
        else
        {
            OSDWindow::getInstance()->showIcon(IMAGE_CAPSLOCK_DISABLED);
        }
    }
    else if (keycode == m_numlockKeycode)
    {
        RETURN_IF_FALSE(m_keyboardManager->getNumlockTipsEnabled());

        bool numlock_enable = !!(m_numlockMask & mods);
        if (numlock_enable)
        {
            OSDWindow::getInstance()->showIcon(IMAGE_NUMLOCK_ENABLED);
        }
        else
        {
            OSDWindow::getInstance()->showIcon(IMAGE_NUMLOCK_DISABLED);
        }
    }
    else
    {
        // others do nothing
    }

    return;
}

bool ModifierLockManager::nativeEventFilter(const QByteArray &eventType, void *message, long *result)
{
    if (eventType == "xcb_generic_event_t")
    {
        xcb_generic_event_t *ev = static_cast<xcb_generic_event_t *>(message);
        if ((ev->response_type & ~0x80) == m_xkbEventBase + XkbEventCode)
        {
            auto xkbEvent = reinterpret_cast<XkbEvent *>(ev);
            if (xkbEvent->any.xkbType == XkbStateNotify &&
                xkbEvent->stateNotify.changed & XkbModifierLockMask)
            {
                setLockAction(xkbEvent->stateNotify.keycode, xkbEvent->stateNotify.lockedMods);
            }
        }
    }

    return false;
}

}  // namespace Kiran