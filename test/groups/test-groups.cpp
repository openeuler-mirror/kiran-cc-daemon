/**
 * Copyright (c) 2026 ~ 2027 KylinSec Co., Ltd.
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

#include "test-groups.h"
#include <grp.h>
#include <sys/types.h>
#include <QDBusConnection>
#include "accounts-i.h"
#include "groups-i.h"
#include "plugins/accounts/accounts-plugin.h"
#include "plugins/groups/groups-plugin.h"
#include "test/groups/accounts_interface.h"
#include "test/groups/group_interface.h"
#include "test/groups/groups_interface.h"
#include "test/groups/user_interface.h"

namespace Kiran
{

#define USRE_NAME_TEST_USER1 "test_user1"
#define USRE_ID_TEST_USER1 5000
#define GROUP_NAME_TEST_GROUP1 "test_group1"

void TestGroups::initTestCase()
{
    qputenv("GSETTINGS_BACKEND", "memory");

    m_groupsPlugin = new GroupsPlugin();
    m_groupsPlugin->activate();
    m_accountsPlugin = new AccountsPlugin();
    m_accountsPlugin->activate();

    m_accountsProxy = new AccountsProxy(ACCOUNTS_DBUS_NAME,
                                        ACCOUNTS_OBJECT_PATH,
                                        QDBusConnection::systemBus(),
                                        this);
    m_groupsProxy = new GroupsProxy(GROUPS_DBUS_NAME,
                                    GROUPS_OBJECT_PATH,
                                    QDBusConnection::systemBus(),
                                    this);
}

void TestGroups::cleanupTestCase()
{
    m_accountsPlugin->deactivate();
    delete m_accountsPlugin;
    m_accountsPlugin = nullptr;

    m_groupsPlugin->deactivate();
    delete m_groupsPlugin;
    m_groupsPlugin = nullptr;
}

void TestGroups::testCreateGroup()
{
    // 函数创建帐户test_user1
    m_accountsProxy->CreateUser(USRE_NAME_TEST_USER1,
                                USRE_NAME_TEST_USER1,
                                AccountsAccountType::ACCOUNTS_ACCOUNT_TYPE_STANDARD,
                                USRE_ID_TEST_USER1)
        .waitForFinished();

    // 创建用户组test_group1，并将test_user1加入用户组
    m_groupsProxy->CreateGroup(GROUP_NAME_TEST_GROUP1, QStringList{USRE_NAME_TEST_USER1})
        .waitForFinished();

    // 调用getgrnam获取用户组test_group1信息
    auto group = getgrnam(GROUP_NAME_TEST_GROUP1);
    QVERIFY(group != nullptr);
    QVERIFY(strcmp(group->gr_name, GROUP_NAME_TEST_GROUP1) == 0);

    // 删除用户组test_group1和帐户test_user1
    m_groupsProxy->DeleteGroup(group->gr_gid).waitForFinished();
    m_accountsProxy->DeleteUser(USRE_ID_TEST_USER1, true).waitForFinished();

    // 调用getgrnam获取用户组test_group1信息
    group = getgrnam(GROUP_NAME_TEST_GROUP1);
    QVERIFY(group == nullptr);
}

}  // namespace Kiran

QTEST_MAIN(Kiran::TestGroups)

#include "test-groups.moc"