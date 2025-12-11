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

#pragma once

#include <qt5-log-i.h>
#include <QLoggingCategory>

namespace Kiran
{
Q_DECLARE_LOGGING_CATEGORY(accounts)
Q_DECLARE_LOGGING_CATEGORY(groups)
Q_DECLARE_LOGGING_CATEGORY(appearance)
Q_DECLARE_LOGGING_CATEGORY(audio)
Q_DECLARE_LOGGING_CATEGORY(clipboard)
Q_DECLARE_LOGGING_CATEGORY(display)
Q_DECLARE_LOGGING_CATEGORY(greeter)
Q_DECLARE_LOGGING_CATEGORY(keyboard)
Q_DECLARE_LOGGING_CATEGORY(mouse)
Q_DECLARE_LOGGING_CATEGORY(touchpad)
Q_DECLARE_LOGGING_CATEGORY(keybinding)
Q_DECLARE_LOGGING_CATEGORY(power)
Q_DECLARE_LOGGING_CATEGORY(systeminfo)
Q_DECLARE_LOGGING_CATEGORY(timedate)
Q_DECLARE_LOGGING_CATEGORY(settings)
Q_DECLARE_LOGGING_CATEGORY(upgrade)

}  // namespace Kiran
