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

#include "xkb-help.h"
#include "lib/base/base.h"

#include <X11/XKBlib.h>
#include <X11/keysymdef.h>
#include <QX11Info>

namespace Kiran
{
bool XkbHelp::xkbSupported(int &xkbEventBase)
{
    if (!QX11Info::isPlatformX11())
    {
        return false;
    }

    int major = XkbMajorVersion;
    int minor = XkbMinorVersion;

    if (!XkbLibraryVersion(&major, &minor))
    {
        KLOG_WARNING(keyboard) << "XKB extension " << major << '.' << minor << " != " << XkbMajorVersion << '.' << XkbMinorVersion;
        return false;
    }

    int opcodeReturn;
    int errorBaseReturn;
    if (!XkbQueryExtension(QX11Info::display(),
                           &opcodeReturn,
                           &xkbEventBase,
                           &errorBaseReturn,
                           &major,
                           &minor))
    {
        KLOG_WARNING(keyboard) << "XKB extension " << major << '.' << minor << " != " << XkbMajorVersion << '.' << XkbMinorVersion;
        return false;
    }

    return true;
}

}  // namespace Kiran
