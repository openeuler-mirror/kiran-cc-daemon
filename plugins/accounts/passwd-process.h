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

#include <QDBusMessage>
#include <QObject>
#include <QProcess>
#include <QWeakPointer>

class QTimer;

namespace Kiran
{
enum PasswdState
{
    PASSWD_STATE_NONE,
    // 请求当前密码
    PASSWD_STATE_AUTH,
    // 请求新密码
    PASSWD_STATE_NEW,
    // 完成两次密码输入
    PASSWD_STATE_RETYPE,
    // 设置密码出现错误
    PASSWD_STATE_ERROR,
    // 运行结束
    PASSWD_STATE_END
};

class User;

class PasswdProcess : public QProcess
{
    Q_OBJECT

public:
    PasswdProcess(QWeakPointer<User> user);
    virtual ~PasswdProcess();

    // 设置密码当前进度/状态
    PasswdState getState() { return this->m_state; };
    // 设置调用者
    void setDBusCaller(uint32_t dbusCallerUID);
    // 执行修改密码命令
    void changePassword(const QString &userName,
                        const QString &currentPassword,
                        const QString &newPassword);

Q_SIGNALS:
    // 命令执行完成信号
    void finished(const QString &errorMessage);

public Q_SLOTS:
    // 响应passwd命令标准输出和错误，将输出信息添加到换成
    void processStandardOutput();
    void processStandardError();
    void processPasswdEnd(int exitCode, QProcess::ExitStatus exitStatus);
    // 中止进程执行，例如密码设置失败，超时执行
    void stopPasswd();

protected:
    virtual void setupChildProcess();

private:
    // 按行处理passwd命令输出
    void processPasswdOutputLine();
    // 如果sbustrs中任何一个字符串是str的子串则返回true，否则返回false
    bool containsOneofSubstrs(const QString &str, const QStringList &substrs);
    // 如果sbustrs中所有字符串都是str的子串则返回true，否则返回false
    bool containsAllofSubstrs(const QString &str, const QStringList &substrs);

    // 对错误提示信息进行翻译
    QString translationPasswdTips(const QString &passwdTips);
    // 对错误信息进行分段，尝试使用pam相关翻译文件进行翻译
    QString translationWithGettext(const QString &messageID);
    void freeResources();

private:
    QWeakPointer<User> m_user;
    uint32_t m_dbusCallerUID;
    PasswdState m_state;
    // 剩余未处理的passwd命令输出数据
    QString m_bufferedPrompt;
    // 命令运行超时定时器
    QTimer *m_passwdTimeout;
    QString m_currentPassword;
    QString m_newPassword;
    QString m_additionalErrorMessage;
    QString m_errorMessage;
};
}  // namespace Kiran