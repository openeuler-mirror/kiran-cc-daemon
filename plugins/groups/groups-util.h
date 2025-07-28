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

#include "lib/base/base.h"

class QDBusMessage;

namespace Kiran
{
#define SPAWN_WITH_DBUS_MESSAGE(message, program, arguments)                              \
    {                                                                                     \
        QString errMessage;                                                               \
        if (!GroupsUtil::spawnWithLoginUid(message, program, arguments, errMessage))      \
        {                                                                                 \
            auto replyMessage = message.createErrorReply(QDBusError::Failed, errMessage); \
            QDBusConnection::systemBus().send(replyMessage);                              \
            return;                                                                       \
        }                                                                                 \
    }

class GroupsUtil : public QObject
{
    Q_OBJECT
public:
    GroupsUtil() {};
    virtual ~GroupsUtil() {};

    static bool getCallerPID(const QDBusMessage &message, uint32_t &pid);
    static bool getCallerUID(const QDBusMessage &message, uint32_t &uid);
    static void getCallerLoginUID(const QDBusMessage &message, QString &loginUID);
    static bool spawnWithLoginUid(const QDBusMessage &message,
                                  const QString &program,
                                  const QStringList &arguments,
                                  QString &error);
    // 翻译命令行返回的错误码
    static bool parseExitStatus(int32_t exitStatus, CCErrorCode &errorCode);
};
}  // namespace Kiran