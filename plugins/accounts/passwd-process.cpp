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

#include "passwd-process.h"
#include <config.h>
#include <libintl.h>
#include <unistd.h>
#include <QFile>
#include <QProcess>
#include <QRegExp>
#include <QSharedPointer>
#include <QTimer>
#include "accounts-util.h"
#include "lib/base/base.h"
#include "user.h"

namespace Kiran
{
// passwd设置密码错误时运行时间较长，因此这里设置命令超时时间为5秒
#define PASSWD_RUN_TIMEOUT_MSEC 5000

PasswdProcess::PasswdProcess(QWeakPointer<User> user) : m_user(user),
                                                        m_dbusCallerUID(0),
                                                        m_state(PASSWD_STATE_NONE)
{
    m_passwdTimeout = new QTimer(this);
}

PasswdProcess::~PasswdProcess()
{
    freeResources();
}

void PasswdProcess::setDBusCaller(uint32_t dbusCallerUID)
{
    m_dbusCallerUID = dbusCallerUID;
}

void PasswdProcess::changePassword(const QString &userName,
                                   const QString &currentPassword,
                                   const QString &newPassword)
{
    auto user = m_user.toStrongRef();
    if (!user)
    {
        KLOG_WARNING(accounts) << "The user already is destroyed, please check code logic.";
        stopPasswd();
        return;
    }

    m_currentPassword = currentPassword;
    m_newPassword = newPassword;
    connect(m_passwdTimeout, SIGNAL(timeout()), this, SLOT(stopPasswd()));
    m_passwdTimeout->start(PASSWD_RUN_TIMEOUT_MSEC);

    setEnvironment(QStringList{"LANG=C"});
    if (m_dbusCallerUID == user->getUID())
    {
        start("/usr/bin/passwd", QStringList(), QIODevice::ReadWrite);
    }
    else
    {
        start("/usr/bin/passwd", QStringList{userName}, QIODevice::ReadWrite);
    }
    connect(this, SIGNAL(readyReadStandardOutput()), this, SLOT(processStandardOutput()));
    connect(this, SIGNAL(readyReadStandardError()), this, SLOT(processStandardError()));
    connect(this, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(processPasswdEnd(int, QProcess::ExitStatus)));
}

void PasswdProcess::processStandardOutput()
{
    setReadChannel(QProcess::StandardOutput);
    processPasswdOutputLine();
}

void PasswdProcess::processStandardError()
{
    setReadChannel(QProcess::StandardError);
    processPasswdOutputLine();
}

void PasswdProcess::setupChildProcess()
{
    auto user = m_user.toStrongRef();
    if (!user)
    {
        KLOG_WARNING(accounts) << "The user already is destroyed, please check code logic.";
        return;
    }

    // 如果是设置当前用户密码，则需要进行降权
    if (m_dbusCallerUID == user->getUID())
    {
        // 必须先设置gid然后再设置uid，否则在设置uid后已经不是特权用户，无法设置gid
        if (setgid(user->getGID()) != 0 ||
            setuid(user->getUID()) != 0)
        {
            exit(1);
        }
    }
}

void PasswdProcess::processPasswdOutputLine()
{
    RETURN_IF_TRUE(m_state == PasswdState::PASSWD_STATE_END);

    QString line;
    while ((line = readLine()).size() > 0)
    {
        // 记录输出是否被处理
        bool isProcessed = true;
        auto lowercaseLine = line.toLower();
        KLOG_INFO(accounts) << "Process passwd output" << line;

        switch (m_state)
        {
        case PASSWD_STATE_NONE:
            // 设置新密码
            if (lowercaseLine.endsWith("new password: "))
            {
                write(QString("%1\n").arg(m_newPassword).toUtf8());
                m_state = PASSWD_STATE_NEW;
            }
            // 填写当前密码，root用户下不会出现这一步
            else if (lowercaseLine.endsWith("current password: ") ||
                     lowercaseLine.endsWith("(current) unix password: "))
            {
                m_state = PASSWD_STATE_AUTH;
                write(QString("%1\n").arg(m_currentPassword).toUtf8());
            }
            else
            {
                isProcessed = false;
            }
            break;
        case PASSWD_STATE_AUTH:
            if (containsOneofSubstrs(lowercaseLine, QStringList{"password: ", "failure", "wrong", "error"}))
            {
                // 认证成功
                if (containsAllofSubstrs(lowercaseLine, QStringList{"password: ", "new"}))
                {
                    write(QString("%1\n").arg(m_newPassword).toUtf8());
                    m_state = PASSWD_STATE_NEW;
                }
                // 认证失败，再次认证
                else if (lowercaseLine.endsWith("current password: "))
                {
                    write(QString("%1\n").arg(m_currentPassword).toUtf8());
                }
                else
                {
                    m_additionalErrorMessage = translationPasswdTips(line);
                }
            }
            break;
        case PASSWD_STATE_NEW:
            if (lowercaseLine.endsWith("retype new password: "))
            {
                m_state = PASSWD_STATE_RETYPE;
                write(QString("%1\n").arg(m_newPassword).toUtf8());
            }
            /* 执行到这里说明设置的密码不符合复杂度要求，但由于PG版本和主版本的PAM配置不一样，所以这里的流程可能存在差异。
               在主版本中，虽然密码复杂度不符合要求，但还是运行进行修改，输出的信息只是一个告警信息。在PG版本中，输出同样
               内容的信息则为错误消息，即不允许设置不符合复杂度要求的密码。但无法根据当前这条信息判断到底执行的是哪个策略，
               因此这里只能做一个记录，需要根据下一条输出信息进行判断，如果下一条信息是一个prompt提示信息，则说明当前属于
               告警信息，否则是错误信息。*/
            else if (line.indexOf('\n') >= 0)
            {
                m_state = PASSWD_STATE_ERROR;
                m_additionalErrorMessage = translationPasswdTips(line);
            }
            else
            {
                isProcessed = false;
            }
            break;
        case PASSWD_STATE_RETYPE:
            if (containsOneofSubstrs(lowercaseLine, QStringList{"successfully"}))
            {
                // 密码设置成功
                m_state = PASSWD_STATE_END;
            }
            else if (containsOneofSubstrs(lowercaseLine, QStringList{"failure"}) ||
                     containsAllofSubstrs(lowercaseLine, QStringList{"password", "already", "used"}))
            {
                m_additionalErrorMessage = translationPasswdTips(line);
                m_state = PASSWD_STATE_ERROR;
            }
            else
            {
                isProcessed = false;
            }
            break;
        case PASSWD_STATE_ERROR:
        {
            if (lowercaseLine.endsWith("retype new password: "))
            {
                // 这里说明上一条信息是告警消息而非错误消息，因此清空错误消息并继续往下走
                m_state = PASSWD_STATE_RETYPE;
                write(QString("%1\n").arg(m_newPassword).toUtf8());
                m_additionalErrorMessage.clear();
            }
            else
            {
                // 到这里说明密码不符合复杂度要求，此时强行退出进程，不再继续执行后续步骤
                m_state = PASSWD_STATE_END;
                stopPasswd();
                return;
            }
            break;
        }
        default:
            isProcessed = false;
            break;
        }

        if (!isProcessed)
        {
            KLOG_INFO(accounts) << "Line" << line << "is ignored";
        }
    }
}

bool PasswdProcess::containsOneofSubstrs(const QString &str, const QStringList &substrs)
{
    for (auto &substr : substrs)
    {
        if (str.contains(substr))
        {
            return true;
        }
    }
    return false;
}

bool PasswdProcess::containsAllofSubstrs(const QString &str, const QStringList &substrs)
{
    for (auto &substr : substrs)
    {
        if (!str.contains(substr))
        {
            return false;
        }
    }
    return true;
}

void PasswdProcess::processPasswdEnd(int exitCode, QProcess::ExitStatus exitStatus)
{
    KLOG_INFO(accounts) << "Process passwd exit, exit code is" << exitCode;

    if (!m_additionalErrorMessage.isEmpty())
    {
        m_errorMessage = KCD_ERROR2STR(CCErrorCode::ERROR_ACCOUNTS_USER_MODIFY_PASSWORD_FAILED).arg(m_additionalErrorMessage);
    }

    if (exitStatus == QProcess::CrashExit && m_errorMessage.isEmpty())
    {
        m_errorMessage = KCD_ERROR2STR(CCErrorCode::ERROR_FAILED);
    }

    Q_EMIT finished(m_errorMessage);
    freeResources();
}

void PasswdProcess::stopPasswd()
{
    /* 在收到passwd的错误消息后，大概会等1-2秒进程才会退出，如果此时超时定时器被触发，
    应该要判断一下是否是已经处理结束，如果是的话，使用之前的错误消息*/
    if (m_state != PasswdState::PASSWD_STATE_END)
    {
        KLOG_WARNING(accounts) << "Passwd run timeout.";
        m_additionalErrorMessage = tr("Password modification timeout.");
        m_state = PasswdState::PASSWD_STATE_END;
    }
    kill();
}

QString PasswdProcess::translationPasswdTips(const QString &passwdTips)
{
    auto trimPasswdTips = passwdTips.trimmed();

    if (trimPasswdTips.startsWith("BAD PASSWORD: "))
    {
        trimPasswdTips = trimPasswdTips.right(trimPasswdTips.length() - strlen("BAD PASSWORD: "));
    }

    if (trimPasswdTips.startsWith("passwd: "))
    {
        trimPasswdTips = trimPasswdTips.right(trimPasswdTips.length() - strlen("passwd: "));
    }

    KLOG_INFO(accounts) << "Trim passwd:" << trimPasswdTips;

    bool translationSuccess = true;
    auto trimPasswdTipsParts = trimPasswdTips.split('-');
    for (int i = 0; i < trimPasswdTipsParts.size(); ++i)
    {
        auto trimPasswdTipsPart = trimPasswdTipsParts[i].trimmed();
        trimPasswdTipsParts[i] = translationWithGettext(trimPasswdTipsPart);
        if (trimPasswdTipsParts[i].isEmpty())
        {
            translationSuccess = false;
            break;
        }
    }

    if (translationSuccess)
    {
        return trimPasswdTipsParts.join(" - ");
    }

#define MATCH_WITH_ONE_NUMBER(pattern, translation)                      \
    {                                                                    \
        auto regex = QRegExp(pattern);                                   \
        if (regex.indexIn(passwdTips) >= 0 && regex.captureCount() == 1) \
        {                                                                \
            auto number = regex.cap(1);                                  \
            return translation.arg(number);                              \
        }                                                                \
    }

    MATCH_WITH_ONE_NUMBER(QString("less than ([0-9]+) character classes"),
                          tr("The password contains less than %1 character classes."));
    MATCH_WITH_ONE_NUMBER(QString("less than ([0-9]+) digits"),
                          tr("The password contains less than %1 digits."));
    MATCH_WITH_ONE_NUMBER(QString("less than ([0-9]+) lowercase letters"),
                          tr("The password contains less than %1 lowercase letters."));
    MATCH_WITH_ONE_NUMBER(QString("less than ([0-9]+) non-alphanumeric characters"),
                          tr("The password contains less than %1 non-alphanumeric characters."));
    MATCH_WITH_ONE_NUMBER(QString("less than ([0-9]+) uppercase letters"),
                          tr("The password contains less than %1 uppercase letters."));
    MATCH_WITH_ONE_NUMBER(QString("monotonic sequence longer than ([0-9]+) characters"),
                          tr("The password contains monotonic sequence longer than %1 characters."));
    MATCH_WITH_ONE_NUMBER(QString("more than ([0-9]+) characters of the same class consecutively"),
                          tr("The password contains more than %1 characters of the same class consecutively."));
    MATCH_WITH_ONE_NUMBER(QString("more than ([0-9]+) same characters consecutively"),
                          tr("The password contains more than %1 same characters consecutively."));
    MATCH_WITH_ONE_NUMBER(QString("shorter than ([0-9]+) characters"),
                          tr("The password is shorter than %1 characters."));

    return trimPasswdTips;
}

QString PasswdProcess::translationWithGettext(const QString &messageID)
{
    KLOG_INFO(accounts) << "Translation message" << messageID << "with gettext.";

#define TRANS_WITH_DOMAIN(domainname, text)                                     \
    do                                                                          \
    {                                                                           \
        BREAK_IF_TRUE(bindtextdomain(domainname, "/usr/share/locale") == NULL); \
        BREAK_IF_TRUE(bind_textdomain_codeset(domainname, "UTF-8") == NULL);    \
        auto translatedText = dgettext(domainname, text.toUtf8().data());       \
        BREAK_IF_TRUE(translatedText == text.toUtf8().data())                   \
        return translatedText;                                                  \
    } while (0)

    TRANS_WITH_DOMAIN("libpwquality", messageID);
    TRANS_WITH_DOMAIN("Linux-PAM", messageID);
    TRANS_WITH_DOMAIN("cracklib", messageID);

    return QString();
}

void PasswdProcess::freeResources()
{
    m_state = PASSWD_STATE_NONE;
    m_bufferedPrompt.clear();
    m_newPassword.clear();
    m_additionalErrorMessage.clear();
    m_errorMessage.clear();
    m_passwdTimeout->stop();
    terminate();
}
}  // namespace Kiran
