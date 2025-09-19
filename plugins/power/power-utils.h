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

#pragma once

#include <QObject>

namespace Kiran
{
class PowerUtils : public QObject
{
    Q_OBJECT

public:
    PowerUtils() {};
    virtual ~PowerUtils() {};

    static QString getTimeTranslation(uint32_t seconds);
    static QString actionEnum2str(uint32_t action);
    static QString eventActionEnum2Str(int eventAction);
    static int eventActionStr2Enum(QString eventActionStr);
    static QString monitorActionEnum2Str(int monitorAction);
    static int monitorActionStr2Enum(QString monitorActionStr);
    static QString computerActionEnum2Str(int computerAction);
    static int computerActionStr2Enum(QString computerActionStr);
    static QString eventEnum2Str(uint32_t event);
    static QString deviceEnum2Str(uint32_t device);
    static QString supplyEnum2Str(uint32_t supply);
};
}  // namespace Kiran