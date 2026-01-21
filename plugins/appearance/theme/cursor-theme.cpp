/**
 * Copyright (c) 2020 ~ 2026 KylinSec Co., Ltd.
 * kiran-cc-daemon is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     liuxinhao <liuxinhao@kylinsec.com.cn>
 */
#include "cursor-theme.h"
#include <QX11Info>
#include <QFile>

#ifdef HAVE_XCURSOR
#include <X11/Xcursor/Xcursor.h>
#endif

namespace Kiran
{
namespace CursorTheme
{
void applyCursorTheme(const QString &theme, int size)
{
#ifdef HAVE_XCURSOR
    // see also: plasma-desktop cursor-theme.cpp
    if (!QX11Info::isPlatformX11())
    {
        return;
    }

    auto display = QX11Info::display();
    // 应用光标主题
    if (!theme.isEmpty()) 
    {
        XcursorSetTheme(display, QFile::encodeName(theme));
    }

    // 设置光标大小
    if (size >= 0) 
    {
        XcursorSetDefaultSize(display, size);
    }

    // 加载默认光标并应用到根窗口
    Cursor handle = XcursorLibraryLoadCursor(display, "left_ptr");
    XDefineCursor(display, DefaultRootWindow(display), handle);
    XFreeCursor(display, handle);
    XFlush(display);
#endif
}
}
}  // namespace Kiran