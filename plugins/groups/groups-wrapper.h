/**
 * Copyright (c) 2020 ~ 2025 KylinSec Co., Ltd.
 * kiran-cc-daemon is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     yuanxing <yuanxing@kylinsec.com.cn>
 */

#pragma once

#include <grp.h>
#include <pwd.h>
#include <QFileSystemWatcher>
#include <QMap>
#include <QObject>
#include <QTimer>

namespace Kiran
{

class GroupEntry
{
public:
    GroupEntry() = delete;
    GroupEntry(struct group *grp);

    QString name;    /* Group name.	*/
    QString passwd;  /* Password.	*/
    qulonglong gid;  /* Group ID.	*/
    QStringList mem; /* Member list.	*/
    bool localGroup;
    bool primaryGroup;
};

class GroupsWrapper : public QObject
{
    Q_OBJECT

public:
    GroupsWrapper();

    static GroupsWrapper *getInstance() { return m_instance; };
    static void globalInit();
    static void globalDeinit() { delete m_instance; };

    QMap<qulonglong, QSharedPointer<GroupEntry>> getGroups();
    QSharedPointer<GroupEntry> getGroupEntryByID(qulonglong gid);
    QSharedPointer<GroupEntry> getGroupEntryByName(QString name);
    bool isUserExist(const QString &userName);

signals:
    void groupsChanged();

private slots:
    void idleReload(const QString &filePath);
    void reload();

private:
    void init();
    void reloadPrimaryGroup();
    void reloadGroups();

private:
    static GroupsWrapper *m_instance;
    QMap<qulonglong, QSharedPointer<GroupEntry>> m_groups;
    QStringList m_pwUsers;

    QFileSystemWatcher *m_fsWatcher;
    QTimer *m_reloadTimer;
};
}  // namespace Kiran
