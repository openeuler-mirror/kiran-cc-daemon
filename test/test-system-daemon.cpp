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

#include <accounts-i.h>
#include <QDBusObjectPath>
#include <QScopedPointer>
#include <QtTest/QtTest>
#include "lib/base/base.h"
#include "lib/base/crypto-helper.h"
#include "test/accounts_interface.h"
#include "test/user_interface.h"

namespace Kiran
{
#define USRE_NAME_TEST001 "test001"

class TestSystemDaemon : public QObject
{
    Q_OBJECT

public:
    TestSystemDaemon(){};
    virtual ~TestSystemDaemon(){};

private slots:
    void initTestCase();
    void createUser();
    void encryptAndDecrypt();

private:
    AccountsProxy *m_accountsProxy;
};

void TestSystemDaemon::initTestCase()
{
    this->m_accountsProxy = new AccountsProxy(ACCOUNTS_DBUS_NAME,
                                              ACCOUNTS_OBJECT_PATH,
                                              QDBusConnection::systemBus(), this);
}

void TestSystemDaemon::createUser()
{
    // Glib::RefPtr<Kiran::SystemDaemon::Accounts::UserProxy> user_proxy;

    // 删除test001用户
    auto userObjectPath = this->m_accountsProxy->FindUserByName(USRE_NAME_TEST001).value();
    QScopedPointer<UserProxy> userProxy(new UserProxy(ACCOUNTS_DBUS_NAME,
                                                      userObjectPath.path(),
                                                      QDBusConnection::systemBus()));

    auto userUID = userProxy->uid();
    this->m_accountsProxy->DeleteUser(userUID, true).waitForFinished();

    this->m_accountsProxy->CreateUser(USRE_NAME_TEST001,
                                      USRE_NAME_TEST001,
                                      AccountsAccountType::ACCOUNTS_ACCOUNT_TYPE_STANDARD,
                                      -1)
        .waitForFinished();

    QCOMPARE(USRE_NAME_TEST001, userProxy->user_name());
}

void TestSystemDaemon::encryptAndDecrypt()
{
    QString rawText("Hello world!");

    QString rsaPrivateKey;
    QString rsaPublicKey;

    Kiran::CryptoHelper::generateRSAKey(512, rsaPrivateKey, rsaPublicKey);
    auto encryptedText = Kiran::CryptoHelper::rsaEncrypt(rsaPublicKey, rawText);
    auto decryptedText = Kiran::CryptoHelper::rsaDecrypt(rsaPrivateKey, encryptedText);

    // printf("private: %s, public: %s, encrypted_text: %s\n",
    //        rsa_private_key.c_str(),
    //        rsa_public_key.c_str(),
    //        encrypted_text.c_str());

    QCOMPARE(rawText, decryptedText);
}

}  // namespace Kiran

QTEST_MAIN(Kiran::TestSystemDaemon)
#include "test-system-daemon.moc"