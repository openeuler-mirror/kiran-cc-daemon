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

#include "test-timedate.h"
#include <QDBusConnection>
#include "plugins/timedate/dbus-types.h"
#include "plugins/timedate/timedate-plugin.h"
#include "test/timedate/timedate_interface.h"
#include "timedate-i.h"

namespace Kiran
{

#define USRE_NAME_TEST001 "test001"
#define USRE_ID_TEST001 5000

void TestTimedate::initTestCase()
{
    qDBusRegisterMetaType<DBusZoneInfos>();
    qDBusRegisterMetaType<DBusZoneInfo>();

    m_plugin = new TimedatePlugin();
    m_plugin->activate();
    m_timedateProxy = new TimedateProxy(TIMEDATE_DBUS_NAME,
                                        TIMEDATE_OBJECT_PATH,
                                        QDBusConnection::systemBus(),
                                        this);
}

void TestTimedate::cleanupTestCase()
{
    m_plugin->deactivate();
    delete m_plugin;
    m_plugin = nullptr;
}

void TestTimedate::testSetTime()
{
    m_timedateProxy->SetTime(QDateTime::currentDateTime().toMSecsSinceEpoch(), false).waitForFinished();
    auto systemTime = m_timedateProxy->system_time();
    // 获取系统时间， 与当前时间对比， 误差不超过50ms
    QVERIFY(qAbs(QDateTime::currentDateTime().toMSecsSinceEpoch() - systemTime / 1000) <= 50000);
}

void TestTimedate::testSetTimezone()
{
    // 备份原来的时区
    auto oldTimeZone = m_timedateProxy->time_zone();

    // 设置新的时区
    m_timedateProxy->SetTimezone("Asia/Beijing").waitForFinished();
    auto newTimeZone = m_timedateProxy->time_zone();
    QVERIFY(newTimeZone == "Asia/Beijing");

    // 恢复原来的时区
    m_timedateProxy->SetTimezone(oldTimeZone).waitForFinished();
}

void TestTimedate::testNTP()
{
    // 备份原来的NTP状态
    auto oldNTP = m_timedateProxy->ntp();

    // 调用TimedateManager::SetNTP开启时间同步服务，通过timedatectl show来验证
    m_timedateProxy->SetNTP(true).waitForFinished();
    QProcess process;
    process.start("timedatectl", QStringList() << "show");
    process.waitForFinished();
    QVERIFY(process.readAllStandardOutput().contains("\nNTP=yes"));

    // 调用TimedateManager::SetNTP关闭时间同步服务，通过timedatectl show来验证
    m_timedateProxy->SetNTP(false).waitForFinished();
    process.start("timedatectl", QStringList() << "show");
    process.waitForFinished();
    QVERIFY(process.readAllStandardOutput().contains("\nNTP=no"));

    // 恢复原来的NTP状态
    m_timedateProxy->SetNTP(oldNTP).waitForFinished();
}  // namespace Kiran

}  // namespace Kiran

QTEST_MAIN(Kiran::TestTimedate)

#include "test-timedate.moc"