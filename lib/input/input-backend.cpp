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

#include "input-backend.h"
#include <QGuiApplication>
#include "backends/wayland/wayland-kwin-backend.h"
#include "backends/x11/xinput-backend.h"

namespace Kiran
{
InputBackend::InputBackend()
{
}

InputBackend* InputBackend::m_instance = nullptr;
void InputBackend::globalInit()
{
    if (qGuiApp->platformName() == "wayland")
    {
        m_instance = new WaylandKwinBackend();
    }
    else
    {
        m_instance = new XInputBackend();
    }
}

}  // namespace Kiran
