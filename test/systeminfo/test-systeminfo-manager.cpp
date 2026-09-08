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
 */

#include "test/systeminfo/test-systeminfo-manager.h"
#include "lib/base/base.h"

#include <systeminfo-i.h>

#include <string>
#include <unistd.h>

namespace Kiran
{

#define HOST_NAME_TEST "hostname-test"

void SystemInfoManagerProxy::SetUp()
{
    this->systeminfo_proxy_ = SystemDaemon::SystemInfoProxy::createForBus_sync(
        Gio::DBus::BUS_TYPE_SYSTEM,
        Gio::DBus::PROXY_FLAGS_NONE,
        SYSTEMINFO_DBUS_NAME,
        SYSTEMINFO_OBJECT_PATH);

    ASSERT_NE(!this->systeminfo_proxy_, true);

    char hostname[256] = {0};
    if (gethostname(hostname, sizeof(hostname) - 1) == 0)
    {
        this->original_hostname_ = hostname;
        this->snapshot_valid_ = true;
    }
}

void SystemInfoManagerProxy::TearDown()
{
    if (!this->snapshot_valid_ || !this->systeminfo_proxy_)
    {
        return;
    }

    IGNORE_EXCEPTION(
        {
            this->systeminfo_proxy_->SetHostName_sync(this->original_hostname_);
        });
}

TEST_F(SystemInfoManagerProxy, SetHostName)
{
    ASSERT_NO_THROW(this->systeminfo_proxy_->SetHostName_sync(HOST_NAME_TEST));

    char hostname[256] = {0};
    ASSERT_EQ(gethostname(hostname, sizeof(hostname) - 1), 0);
    ASSERT_EQ(std::string(hostname), HOST_NAME_TEST);
}

}  // namespace Kiran
