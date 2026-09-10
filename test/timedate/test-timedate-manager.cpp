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

#include "test/timedate/test-timedate-manager.h"
#include "lib/base/base.h"

#include <cstdlib>
#include <ctime>
#include <sstream>
#include <string>
#include <sys/time.h>
#include <timedate-i.h>
#include <unistd.h>

namespace Kiran
{

static std::string localtime_zone()
{
    char buf[512] = {0};
    ssize_t len = readlink("/etc/localtime", buf, sizeof(buf) - 1);
    if (len <= 0)
    {
        return std::string();
    }
    std::string target(buf, (size_t)len);
    auto pos = target.rfind("zoneinfo/");
    if (pos == std::string::npos)
    {
        return std::string();
    }
    return target.substr(pos + 9);
}

static std::string timedatectl_show()
{
    std::string output;
    gchar *standard_output = nullptr;
    gint exit_status = 0;
    GError *error = nullptr;

    if (g_spawn_command_line_sync("timedatectl show", &standard_output, nullptr, &exit_status, &error))
    {
        if (standard_output)
        {
            output = standard_output;
            g_free(standard_output);
        }
    }
    if (error)
    {
        g_error_free(error);
    }
    return output;
}

static std::string timedatectl_value(const std::string &key)
{
    std::istringstream stream(timedatectl_show());
    std::string line;
    while (std::getline(stream, line))
    {
        if (line.compare(0, key.size(), key) == 0)
        {
            return line.substr(key.size());
        }
    }
    return std::string();
}

static guint64 now_us()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (guint64)tv.tv_sec * 1000000 + tv.tv_usec;
}

void TimedateManagerProxy::SetUp()
{
    this->timedate_proxy_ = SystemDaemon::TimeDateProxy::createForBus_sync(
        Gio::DBus::BUS_TYPE_SYSTEM,
        Gio::DBus::PROXY_FLAGS_NONE,
        TIMEDATE_DBUS_NAME,
        TIMEDATE_OBJECT_PATH);

    ASSERT_NE(!this->timedate_proxy_, true);

    this->original_timezone_ = localtime_zone();
    this->original_ntp_ = (timedatectl_value("NTP=") == "yes");
    this->snapshot_valid_ = !this->original_timezone_.empty();
}

void TimedateManagerProxy::TearDown()
{
    if (!this->snapshot_valid_ || !this->timedate_proxy_)
    {
        return;
    }

    IGNORE_EXCEPTION(
        {
            this->timedate_proxy_->SetTimezone_sync(this->original_timezone_);
            if (timedatectl_value("NTP=") == "yes")
            {
                this->timedate_proxy_->SetNTP_sync(false);
            }
            this->timedate_proxy_->SetTime_sync((gint64)now_us(), false);
            this->timedate_proxy_->SetNTP_sync(this->original_ntp_);
        });
}

TEST_F(TimedateManagerProxy, SetTime)
{
    if (timedatectl_value("NTP=") == "yes")
    {
        ASSERT_NO_THROW(this->timedate_proxy_->SetNTP_sync(false));
    }

    guint64 requested_us = now_us() + 5000000;
    ASSERT_NO_THROW(this->timedate_proxy_->SetTime_sync(requested_us, false));

    // 拨钟可能非立即生效：以 now+5s 为基准，≤5s 内轮询系统时钟，命中目标(±2s)即成功，超时判未生效
    bool applied = false;
    for (int i = 0; i < 50; ++i)
    {
        guint64 current_us = now_us();
        guint64 diff_us = current_us > requested_us ? current_us - requested_us : requested_us - current_us;
        if (diff_us <= 2000000)
        {
            applied = true;
            break;
        }
        g_usleep(100000);
    }

    ASSERT_NO_THROW(this->timedate_proxy_->SetTime_sync((gint64)now_us(), false));

    ASSERT_TRUE(applied);
}

TEST_F(TimedateManagerProxy, SetTimezone)
{
    ASSERT_NO_THROW(this->timedate_proxy_->SetTimezone_sync("Asia/Beijing"));
    ASSERT_EQ(localtime_zone(), "Asia/Beijing");
}

TEST_F(TimedateManagerProxy, NTP)
{
    if (!this->timedate_proxy_->can_ntp_get())
    {
        GTEST_SKIP() << "NTP not available on this system";
    }

    ASSERT_NO_THROW(this->timedate_proxy_->SetNTP_sync(true));
    ASSERT_EQ(timedatectl_value("NTP="), "yes");

    ASSERT_NO_THROW(this->timedate_proxy_->SetNTP_sync(false));
    ASSERT_EQ(timedatectl_value("NTP="), "no");
}

}  // namespace Kiran
