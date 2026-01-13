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

#include "test-systeminfo.h"
#include <QDBusConnection>
#include "plugins/systeminfo/systeminfo-plugin.h"
#include "systeminfo-i.h"
#include "test/systeminfo/systeminfo_interface.h"

namespace Kiran
{

#define HOST_NAME_TEST "hostname-test"

void TestSystemInfo::initTestCase()
{
    m_plugin = new SystemInfoPlugin();
    m_plugin->activate();
    m_systeminfoProxy = new SysteminfoProxy(SYSTEMINFO_DBUS_NAME,
                                            SYSTEMINFO_OBJECT_PATH,
                                            QDBusConnection::systemBus(),
                                            this);
}

void TestSystemInfo::cleanupTestCase()
{
    m_plugin->deactivate();
    delete m_plugin;
    m_plugin = nullptr;
}

void TestSystemInfo::testSetHostName()
{
    // 备份原来的主机名
    auto oldHostName = m_systeminfoProxy->host_name();

    // 设置新的主机名
    m_systeminfoProxy->SetHostName(HOST_NAME_TEST).waitForFinished();
    auto newHostName = m_systeminfoProxy->host_name();
    QVERIFY(newHostName == HOST_NAME_TEST);

    // 恢复原来的主机名
    m_systeminfoProxy->SetHostName(oldHostName).waitForFinished();
}

}  // namespace Kiran

QTEST_MAIN(Kiran::TestSystemInfo)

#include "test-systeminfo.moc"