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

#include <QList>
#include <QObject>
#include <QSharedPointer>

namespace Kiran
{

class InputDevice;

class InputBackend : public QObject
{
    Q_OBJECT

public:
    InputBackend();
    virtual ~InputBackend(){};

    static InputBackend* getDefault();

    virtual bool isValid() { return false; };

    virtual QList<QSharedPointer<InputDevice>> getDevices() const { return QList<QSharedPointer<InputDevice>>(); };

private:
    static InputBackend* m_instance;
};

}  // namespace Kiran
