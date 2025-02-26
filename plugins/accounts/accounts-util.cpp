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

#include "accounts-util.h"
#include <glib.h>
#include <unistd.h>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QFile>
#include <QProcess>
#include <QTextStream>

enum CommandExitStatus
{
    // success
    COMMAND_EXIT_STATUS_SUCCESS = 0,
    // can't update password file
    COMMAND_EXIT_STATUS_PW_UPDATE = 1,
    // invalid command syntax
    COMMAND_EXIT_STATUS_USAGE = 2,
    // invalid argument to option
    COMMAND_EXIT_STATUS_BAD_ARG = 3,
    // UID already in use (and no -o)
    COMMAND_EXIT_STATUS_UID_IN_USE = 4,
    // passwd file contains errors
    COMMAND_EXIT_STATUS_BAD_PWFILE = 5,
    // specified usr/group doesn't exist
    COMMAND_EXIT_STATUS_NOTFOUND = 6,
    // user to modify is logged in
    COMMAND_EXIT_STATUS_USER_BUSY = 8,
    // username already in use
    COMMAND_EXIT_STATUS_NAME_IN_USE = 9,
    // can't update group file
    COMMAND_EXIT_STATUS_GRP_UPDATE = 10,
    // insufficient space to move home dir
    COMMAND_EXIT_STATUS_NOSPACE = 11,
    // can't create/remove/move home directory
    COMMAND_EXIT_STATUS_HOMEDIR = 12,
    // can't update SELinux user mapping
    COMMAND_EXIT_STATUS_SE_UPDATE_1 = 13,
    // can't update SELinux user mapping
    COMMAND_EXIT_STATUS_SE_UPDATE_2 = 14,
    // can't update the subordinate uid file
    COMMAND_EXIT_STATUS_SUB_UID_UPDATE = 16,
    // can't update the subordinate gid file
    COMMAND_EXIT_STATUS_SUB_GID_UPDATE = 18,

};

namespace Kiran
{
bool AccountsUtil::getCallerPID(const QDBusMessage &message, uint32_t &pid)
{
    auto sendMessage = QDBusMessage::createMethodCall("org.freedesktop.DBus",
                                                      "/org/freedesktop/DBus",
                                                      "org.freedesktop.DBus",
                                                      "GetConnectionUnixProcessID");
    sendMessage << message.service();
    auto replyMessage = QDBusConnection::systemBus().call(sendMessage, QDBus::Block);
    if (replyMessage.type() == QDBusMessage::ErrorMessage)
    {
        KLOG_WARNING(accounts) << "Call GetConnectionUnixProcessID failed: " << replyMessage.errorMessage();
        return false;
    }
    pid = replyMessage.arguments().takeFirst().toUInt();
    return true;
}

bool AccountsUtil::getCallerUID(const QDBusMessage &message, uint32_t &uid)
{
    auto sendMessage = QDBusMessage::createMethodCall("org.freedesktop.DBus",
                                                      "/org/freedesktop/DBus",
                                                      "org.freedesktop.DBus",
                                                      "GetConnectionUnixUser");
    sendMessage << message.service();
    auto replyMessage = QDBusConnection::systemBus().call(sendMessage, QDBus::Block);
    if (replyMessage.type() == QDBusMessage::ErrorMessage)
    {
        KLOG_WARNING(accounts) << "Call GetConnectionUnixUser failed: " << replyMessage.errorMessage();
        return false;
    }
    uid = replyMessage.arguments().takeFirst().toUInt();
    return true;
}

void AccountsUtil::getCallerLoginUID(const QDBusMessage &message, QString &loginUID)
{
    uint32_t pid = 0;
    uint32_t uid = 0;

    if (!AccountsUtil::getCallerUID(message, uid))
    {
        uid = getuid();
    }

    if (AccountsUtil::getCallerPID(message, pid))
    {
        auto path = QString("/proc/%1/loginuid").arg(pid);
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            KLOG_WARNING(accounts) << "Cannot access file " << path;
            return;
        }

        QTextStream in(&file);
        loginUID = in.readLine();
    }

    if (loginUID.isEmpty())
    {
        loginUID = QString("%1").arg(uid);
    }
}

class SetupChildProcess : public QProcess
{
public:
    SetupChildProcess(const QString &loginUID) : m_loginUID(loginUID){};

protected:
    virtual void setupChildProcess()
    {
        QFile file("/proc/self/loginuid");
        if (file.open(QIODevice::ReadWrite | QIODevice::Text))
        {
            QTextStream out(&file);
            out << m_loginUID;
            out.flush();
        }
    };

private:
    QString m_loginUID;
};

bool AccountsUtil::spawnWithLoginUid(const QDBusMessage &message,
                                     const QString &program,
                                     const QStringList &arguments,
                                     QString &error)
{
    QString loginUID;
    CCErrorCode errorCode = CCErrorCode::SUCCESS;

    AccountsUtil::getCallerLoginUID(message, loginUID);

    SetupChildProcess process(loginUID);
    process.start(program, arguments);
    process.waitForFinished();
    auto standardError = process.readAllStandardError();
    auto exitCode = process.exitCode();
    auto command = QString("%1 %2").arg(program).arg(arguments.join(" "));

    KLOG_INFO(accounts) << "The exitcode of command" << command << "is" << exitCode << ", exit status is" << process.exitStatus();

    if (process.exitStatus() == QProcess::CrashExit)
    {
        errorCode = CCErrorCode::ERROR_ACCOUNTS_SPAWN_SYNC_FAILED;
    }
    else
    {
        AccountsUtil::parseExitStatus(exitCode, errorCode);
    }

    if (errorCode != CCErrorCode::SUCCESS)
    {
        error = CCError::getErrorDesc(errorCode, false);
        if (!standardError.isEmpty())
        {
            error.append(tr(" (%1, error code: 0x%2)").arg(QString(standardError.trimmed())).arg(errorCode, 0, 16));
        }
        else
        {
            error.append(tr(" (error code: 0x%1)").arg(errorCode, 0, 16));
        }
        return false;
    }
    return true;
}

bool AccountsUtil::parseExitStatus(int32_t exitStatus, CCErrorCode &errorCode)
{
    GError *gError = NULL;

    if (!WIFEXITED(exitStatus))
    {
#if GLIB_CHECK_VERSION(2, 70, 0)
        auto result = g_spawn_check_wait_status(exitStatus, &gError);
#else
        auto result = g_spawn_check_exit_status(exitStatus, &gError);
#endif

        if (!result)
        {
            KLOG_WARNING(accounts) << gError->message;
            g_error_free(gError);
            errorCode = CCErrorCode::ERROR_ACCOUNTS_SPAWN_EXIT_STATUS;
        }
        return result;
    }
    switch (WEXITSTATUS(exitStatus))
    {
    case COMMAND_EXIT_STATUS_SUCCESS:
        return true;
    case COMMAND_EXIT_STATUS_PW_UPDATE:
        errorCode = CCErrorCode::ERROR_ACCOUNTS_USER_COMMAND_PW_UPDATE;
        break;
    case COMMAND_EXIT_STATUS_USAGE:
        errorCode = CCErrorCode::ERROR_ACCOUNTS_USER_COMMAND_USAGE;
        break;
    case COMMAND_EXIT_STATUS_BAD_ARG:
        errorCode = CCErrorCode::ERROR_ACCOUNTS_USER_COMMAND_BAD_ARG;
        break;
    case COMMAND_EXIT_STATUS_UID_IN_USE:
        errorCode = CCErrorCode::ERROR_ACCOUNTS_USER_COMMAND_UID_IN_USE;
        break;
    case COMMAND_EXIT_STATUS_BAD_PWFILE:
        errorCode = CCErrorCode::ERROR_ACCOUNTS_USER_COMMAND_BAD_PWFILE;
        break;
    case COMMAND_EXIT_STATUS_NOTFOUND:
        errorCode = CCErrorCode::ERROR_ACCOUNTS_USER_COMMAND_NOTFOUND;
        break;
    case COMMAND_EXIT_STATUS_USER_BUSY:
        errorCode = CCErrorCode::ERROR_ACCOUNTS_USER_COMMAND_USER_BUSY;
        break;
    case COMMAND_EXIT_STATUS_NAME_IN_USE:
        errorCode = CCErrorCode::ERROR_ACCOUNTS_USER_COMMAND_NAME_IN_USE;
        break;
    case COMMAND_EXIT_STATUS_GRP_UPDATE:
        errorCode = CCErrorCode::ERROR_ACCOUNTS_USER_COMMAND_GRP_UPDATE;
        break;
    case COMMAND_EXIT_STATUS_NOSPACE:
        errorCode = CCErrorCode::ERROR_ACCOUNTS_USER_COMMAND_NOSPACE;
        break;
    case COMMAND_EXIT_STATUS_HOMEDIR:
        errorCode = CCErrorCode::ERROR_ACCOUNTS_USER_COMMAND_HOMEDIR;
        break;
    case COMMAND_EXIT_STATUS_SE_UPDATE_1:
        errorCode = CCErrorCode::ERROR_ACCOUNTS_USER_COMMAND_SE_UPDATE_1;
        break;
    case COMMAND_EXIT_STATUS_SE_UPDATE_2:
        errorCode = CCErrorCode::ERROR_ACCOUNTS_USER_COMMAND_SE_UPDATE_2;
        break;
    case COMMAND_EXIT_STATUS_SUB_UID_UPDATE:
        errorCode = CCErrorCode::ERROR_ACCOUNTS_USER_COMMAND_SUB_UID_UPDATE;
        break;
    case COMMAND_EXIT_STATUS_SUB_GID_UPDATE:
        errorCode = CCErrorCode::ERROR_ACCOUNTS_USER_COMMAND_SUB_GID_UPDATE;
        break;
    default:
        errorCode = CCErrorCode::ERROR_ACCOUNTS_USER_COMMAND_UNKNOWN;
        break;
    }
    return false;
}

}  // namespace Kiran
