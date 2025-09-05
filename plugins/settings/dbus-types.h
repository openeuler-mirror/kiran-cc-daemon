/**
 * Copyright (c) 2024 ~ 2025 KylinSec Co., Ltd.
 * kiran-control-panel is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     tangjie02 <tangjie02@kylinsec.com.cn>
 */

#pragma once

#include <QDBusArgument>
#include <QList>
#include <QMetaType>
#include <QObject>
#include <QSize>

struct DColor
{
    DColor() : red(0), green(0), blue(0), alpha(0) {}
    DColor(uint16_t r, uint16_t g, uint16_t b, uint16_t a) : red(r), green(g), blue(b), alpha(a) {}
    uint16_t red;
    uint16_t green;
    uint16_t blue;
    uint16_t alpha;

    friend QDBusArgument &operator<<(QDBusArgument &argument, const DColor &dcolor)
    {
        argument.beginStructure();
        argument << dcolor.red << dcolor.green << dcolor.blue << dcolor.alpha;
        argument.endStructure();
        return argument;
    }

    friend const QDBusArgument &operator>>(const QDBusArgument &argument, DColor &dcolor)
    {
        argument.beginStructure();
        argument >> dcolor.red >> dcolor.green >> dcolor.blue >> dcolor.alpha;
        argument.endStructure();
        return argument;
    }
};

Q_DECLARE_METATYPE(DColor)
