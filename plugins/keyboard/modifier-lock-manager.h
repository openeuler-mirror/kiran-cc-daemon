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

#pragma once

#include <QAbstractNativeEventFilter>
#include "keyboard-manager.h"

typedef uint8_t xcb_keycode_t;

namespace Kiran
{
class ModifierLockManager : public QObject, public QAbstractNativeEventFilter
{
    Q_OBJECT

public:
    ModifierLockManager(KeyboardManager *keyboardManager);
    ~ModifierLockManager();

    static ModifierLockManager *getInstance() { return m_instance; };
    static void globalInit(KeyboardManager *keyboardManager);
    static void globalDeinit() { delete m_instance; };

private:
    void init();
    void setLockAction(xcb_keycode_t keycode, uint8_t mods);
    bool nativeEventFilter(const QByteArray &eventType, void *message, long *result) override;

private:
    static ModifierLockManager *m_instance;
    KeyboardManager *m_keyboardManager;
    int m_xkbEventBase;
    unsigned int m_capslockMask;
    unsigned int m_numlockMask;
    xcb_keycode_t m_capslockKeycode;
    xcb_keycode_t m_numlockKeycode;
};

}  // namespace Kiran