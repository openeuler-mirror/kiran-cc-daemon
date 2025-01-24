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

#include <QSharedPointer>
#include "../../input-backend.h"

namespace Kiran
{

class XcbConnection;

class XInputBackend : public InputBackend
{
    Q_OBJECT

public:
    XInputBackend();
    virtual ~XInputBackend(){};

    virtual bool isValid();

    virtual QList<QSharedPointer<InputDevice>> getDevices() const;

private:
    QSharedPointer<XcbConnection> m_xcbConnection;
};

}  // namespace Kiran
