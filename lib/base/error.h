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
#include "error-i.h"

namespace Kiran
{
#define KCD_ERROR2STR(errorCode) CCError::getErrorDesc(errorCode)

#define DBUS_ERROR_REPLY(errorCode, ...)                                                      \
    {                                                                                         \
        auto errMessage = fmt::format(KCD_ERROR2STR(errorCode).toStdString(), ##__VA_ARGS__); \
        sendErrorReply(QDBusError::Failed, QString(errMessage.c_str()));                      \
    }

#define DBUS_ERROR_REPLY_AND_RET(errorCode, ...) \
    DBUS_ERROR_REPLY(errorCode, ##__VA_ARGS__);  \
    return;

#define DBUS_ERROR_REPLY_AND_RETVAL(val, errorCode, ...) \
    DBUS_ERROR_REPLY(errorCode, ##__VA_ARGS__);          \
    return val;

#define DBUS_ERROR_DELAY_REPLY(errorCode, ...)                                                         \
    {                                                                                                  \
        auto errMessage = fmt::format(KCD_ERROR2STR(errorCode).toStdString(), ##__VA_ARGS__);          \
        auto replyMessage = message.createErrorReply(QDBusError::Failed, QString(errMessage.c_str())); \
        QDBusConnection::systemBus().send(replyMessage);                                               \
    }

#define DBUS_ERROR_DELAY_REPLY_AND_RET(errorCode, ...)    \
    {                                                     \
        DBUS_ERROR_DELAY_REPLY(errorCode, ##__VA_ARGS__); \
        return;                                           \
    }

class CCError : public QObject
{
    Q_OBJECT

public:
    CCError();
    virtual ~CCError(){};

    static QString getErrorDesc(CCErrorCode errorCode, bool attachErrorCode = true);
};

}  // namespace Kiran