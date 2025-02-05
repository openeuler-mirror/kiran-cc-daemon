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

#include <QDBusObjectPath>
#include "lib/base/base.h"

namespace Kiran
{
class SessionGuarder : public QObject
{
    Q_OBJECT

public:
    SessionGuarder();
    virtual ~SessionGuarder() {};

    static SessionGuarder* getInstance() { return m_instance; };

    static void globalInit();

    static void globalDeinit() { delete m_instance; };

Q_SIGNALS:
    void sessionEnd();

private:
    void init();

private Q_SLOTS:
    void responseSessionQueryEnd();
    void responseSessionEnd();

private:
    static SessionGuarder* m_instance;
    // 通过会话管理注册的客户端ObjectPath
    QDBusObjectPath m_clientObjectPath;
};
}  // namespace Kiran
