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

#include "test-accounts.h"
#include <pwd.h>
#include <qdbusmessage.h>
#include <qreadwritelock.h>
#include <sys/types.h>
#include <QDBusConnection>
#include "accounts-i.h"
#include "plugins/accounts/accounts-plugin.h"
#include "test/accounts/accounts_interface.h"

namespace Kiran
{

#define USRE_NAME_TEST001 "test001"
#define USRE_ID_TEST001 5000

void TestAccounts::initTestCase()
{
    m_plugin = new AccountsPlugin();
    m_plugin->activate();
    m_accountsProxy = new AccountsProxy(ACCOUNTS_DBUS_NAME,
                                        ACCOUNTS_OBJECT_PATH,
                                        QDBusConnection::systemBus(),
                                        this);
}

void TestAccounts::cleanupTestCase()
{
    m_plugin->deactivate();
    delete m_plugin;
    m_plugin = nullptr;
}

void TestAccounts::testCreateUser()
{
    // 创建test001用户
    auto userObjectPath = m_accountsProxy->CreateUser(USRE_NAME_TEST001,
                                                      USRE_NAME_TEST001,
                                                      AccountsAccountType::ACCOUNTS_ACCOUNT_TYPE_STANDARD,
                                                      USRE_ID_TEST001)
                              .value();

    // 通过getpwnam获取USRE_NAME_TEST001帐号成功
    QVERIFY(getpwnam(USRE_NAME_TEST001) != NULL);

    // 删除test001用户
    m_accountsProxy->DeleteUser(USRE_ID_TEST001, true).waitForFinished();
}

}  // namespace Kiran

QTEST_MAIN(Kiran::TestAccounts)

#include "test-accounts.moc"