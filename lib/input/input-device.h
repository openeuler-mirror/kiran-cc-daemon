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

#include <QObject>

namespace Kiran
{
class InputDevice : public QObject
{
    Q_OBJECT
public:
    // 判断设备是否存在指定的属性名
    virtual bool hasProperty(const QString &name) { return false; };
    // 判断设备是否为触摸板
    virtual bool isTouchpad() { return false; };
    // 设置属性值
    virtual void setProperty(const QString &name, const QVector<bool> &values){};
    virtual void setProperty(const QString &name, float value){};
};

}  // namespace Kiran
