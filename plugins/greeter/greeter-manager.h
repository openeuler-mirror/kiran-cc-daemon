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
 * Author:     songchuanfei <songchuanfei@kylinos.com.cn>
 */

#pragma once

#include <QDBusContext>
#include "greeter-i.h"

class GreeterAdaptor;
class QFileSystemWatcher;

namespace Kiran
{

class GreeterManager : public QObject,
                       protected QDBusContext
{
    Q_OBJECT

    Q_PROPERTY(bool allow_manual_login READ getAllowManualLogin WRITE setAllowManualLogin)
    Q_PROPERTY(qulonglong autologin_timeout READ getAutologinTimeout WRITE setAutologinTimeout)
    Q_PROPERTY(QString autologin_user READ getAutologinUser WRITE setAutologinUser)
    Q_PROPERTY(QString background READ getBackground WRITE setBackground)
    Q_PROPERTY(bool hide_user_list READ getHideUserList WRITE setHideUserList)
    Q_PROPERTY(ushort scale_factor READ getScaleFactor WRITE setScaleFactor)
    Q_PROPERTY(ushort scale_mode READ getScaleMode WRITE setScaleMode)

public:
    GreeterManager();
    virtual ~GreeterManager();
    static GreeterManager *getInstance() { return m_instance; }
    static void globalInit();
    static void globalDeinit() { delete m_instance; }

    bool getAllowManualLogin() const;
    qulonglong getAutologinTimeout() const;
    QString getAutologinUser() const;
    QString getBackground() const;
    bool getHideUserList() const;
    ushort getScaleFactor() const;
    ushort getScaleMode() const;

    void setAllowManualLogin(bool allow);
    void setAutologinTimeout(qulonglong seconds);
    void setAutologinUser(const QString &userName);
    void setBackground(const QString &filePath);
    void setHideUserList(bool hide);
    void setScaleFactor(ushort scaleFactor);
    void setScaleMode(ushort scaleMode);

public Q_SLOTS:
    void SetAllowManualLogin(bool allow);
    void SetAutologinTimeout(qulonglong seconds);
    void SetAutologinUser(const QString &userName);
    void SetBackground(const QString &filePath);
    void SetHideUserList(bool hide);
    void SetScaleMode(ushort mode, ushort factor);

private:
    void setAllowManualLoginAuthenticated(const QDBusMessage &message, bool allow);
    void setAutologinTimeoutAuthenticated(const QDBusMessage &message, qulonglong seconds);
    void setAutologinUserAuthenticated(const QDBusMessage &message, const QString &userName);
    void setBackgroundAuthenticated(const QDBusMessage &message, const QString &filePath);
    void setHideUserListAuthenticated(const QDBusMessage &message, bool hide);
    void setScaleModeAuthenticated(const QDBusMessage &message, ushort mode, ushort factor);

private:
    void init();
    void loadPropsFromSettings();
    int scaleModeStr2Enum(const QString &str);
    QString scaleModeEnum2Str(int scaleMode);

private Q_SLOTS:
    void processSettingsChanged(const QString &path);

private:
    static GreeterManager *m_instance;

    GreeterAdaptor *m_adaptor;
    QFileSystemWatcher *m_settingsWatcher;

    // 缩放模式
    int m_scaleMode;
    // 自动登录延时,单位为秒
    uint32_t m_autologinDelay;
    // 界面缩放比例, 1表示100%, 2表示200%
    uint32_t m_scaleFactor;
    // 是否允许手动输入用户名登录
    bool m_enableManualLogin;
    // 是否隐藏用户列表
    bool m_hideUserList;
    // 自动登录用户名
    QString m_autologinUser;
    // 登录界面背景图片
    QString m_backgroundFile;
};

}  // namespace Kiran