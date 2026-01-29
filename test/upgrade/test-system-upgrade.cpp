/**
 * Copyright (c) 2020 ~ 2026 KylinSec Co., Ltd.
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

#include "test-system-upgrade.h"
#include <upgrade-i.h>
#include <QDBusPendingReply>
#include <QDateTime>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSignalSpy>
#include <QtTest>

#include "plugins/upgrade/upgrade-plugin.h"
#include "test/upgrade/upgrade_interface.h"

#define TEST_REMINDER_INTERVAL 30

namespace Kiran
{
void TestSystemUpgrade::initTestCase()
{
    m_plugin = new UpgradePlugin();
    m_plugin->activate();
    m_upgradeProxy = new UpgradeProxy(UPGRADE_DBUS_NAME,
                                      UPGRADE_OBJECT_PATH,
                                      QDBusConnection::systemBus(),
                                      this);
}

void TestSystemUpgrade::cleanupTestCase()
{
    m_plugin->deactivate();
    delete m_plugin;
    m_plugin = nullptr;
}

void TestSystemUpgrade::testSetReminderInterval()
{
    //获取系统当前reminder_interval值
    auto currentInterval = m_upgradeProxy->reminder_interval();

    //设置新的提醒周期
    m_upgradeProxy->SetReminderInterval(TEST_REMINDER_INTERVAL).waitForFinished();
    auto actualInterval = m_upgradeProxy->reminder_interval();
    QCOMPARE(actualInterval, TEST_REMINDER_INTERVAL);

    //恢复系统值
    m_upgradeProxy->SetReminderInterval(currentInterval).waitForFinished();
}

void TestSystemUpgrade::testScan()
{
    QSignalSpy spy(m_upgradeProxy, SIGNAL(ScanCompleted(bool, QString)));

    //验证扫描接口
    auto scanReply = m_upgradeProxy->Scan();
    scanReply.waitForFinished();
    QVERIFY(!scanReply.isError());

    //验证信号
    QVERIFY(spy.wait(30000));
    QCOMPARE(spy.count(), 1);

    QList<QVariant> arguments = spy.takeFirst();
    QVERIFY(arguments.size() >= 2);

    QVERIFY(arguments.at(0).type() == QVariant::Bool);
    QVERIFY(arguments.at(1).type() == QVariant::String);

    //验证获取可升级包信息接口
    auto reply = m_upgradeProxy->GetUpgradePkgsInfo();
    reply.waitForFinished();
    QVERIFY(!reply.isError());

    // 验证最新扫描时间不为空且小于当前时间
    auto latestScanTime = m_upgradeProxy->latest_scan_time();
    QVERIFY(!latestScanTime.isEmpty());
    QVERIFY(QDateTime::fromString(latestScanTime, DEFAULT_DATE_TIME_FORMAT) <= QDateTime::currentDateTime());
}

void TestSystemUpgrade::testSolveDeps()
{
    QJsonDocument doc;
    QJsonArray array;
    QJsonObject obj;

    QSignalSpy spy(m_upgradeProxy, SIGNAL(SolveDepsCompleted(bool, QString, QString)));

    // 扫描获取可升级包
    m_upgradeProxy->Scan().waitForFinished();
    QTest::qWait(30000);  // 等待扫描完成

    // 获取可升级包信息
    auto pkgsInfoReply = m_upgradeProxy->GetUpgradePkgsInfo();
    pkgsInfoReply.waitForFinished();
    auto pkgsInfo = pkgsInfoReply.value();
    if (pkgsInfo.isEmpty() || pkgsInfo == "[]")
    {
        QSKIP("There are no upgradeable packages, skip the dependency solving test");
    }

    doc = QJsonDocument::fromJson(pkgsInfo.toUtf8());
    array = doc.array();
    obj = array.at(0).toObject();  // 获取第一个包的ID
    // 验证软件包id
    auto id = obj["id"].toString();
    QVERIFY(!id.isEmpty());

    QStringList packageIds = QStringList() << id;
    // 验证解析依赖接口
    auto reply = m_upgradeProxy->SolveDeps(packageIds);
    reply.waitForFinished();
    QVERIFY(!reply.isError());

    // 验证信号
    QVERIFY(spy.wait(30000));
    QCOMPARE(spy.count(), 1);

    QList<QVariant> arguments = spy.takeFirst();
    QVERIFY(arguments.size() >= 3);

    QVERIFY(arguments.at(0).type() == QVariant::Bool);
    QVERIFY(arguments.at(1).type() == QVariant::String);
    QVERIFY(arguments.at(2).type() == QVariant::String);

    // 验证返回的依赖信息
    doc = QJsonDocument::fromJson(arguments.at(1).toString().toUtf8());
    obj = doc.object();
    auto requestPackages = obj["request_packages"].toArray();
    auto dependencyPackages = obj["dependency_packages"].toArray();
    QVERIFY(requestPackages.contains(id));
    QVERIFY(!dependencyPackages.isEmpty());
}

}  // namespace Kiran

QTEST_MAIN(Kiran::TestSystemUpgrade)

#include "test-system-upgrade.moc"
