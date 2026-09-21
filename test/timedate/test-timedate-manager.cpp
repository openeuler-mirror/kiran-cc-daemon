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
 * Author:     gaobo <gaobo@kylinsec.com.cn>
 */

#include "test/timedate/test-timedate-manager.h"
#include "test/gtest-compat.h"
#include "lib/base/base.h"

#include <cstdlib>
#include <ctime>
#include <sstream>
#include <string>
#include <vector>
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

static std::string run_command(const char *command)
{
    std::string output;
    gchar *standard_output = nullptr;
    gint exit_status = 0;
    GError *error = nullptr;

    if (g_spawn_command_line_sync(command, &standard_output, nullptr, &exit_status, &error) && standard_output)
    {
        output = standard_output;
    }
    g_free(standard_output);
    if (error)
    {
        g_error_free(error);
    }
    return output;
}

static std::string trim(const std::string &text)
{
    const char *spaces = " \t\r\n";
    auto begin = text.find_first_not_of(spaces);
    if (begin == std::string::npos)
    {
        return std::string();
    }
    auto end = text.find_last_not_of(spaces);
    return text.substr(begin, end - begin + 1);
}

// NTP 单元来自 /etc/systemd/ntp-units.d、/usr/lib/systemd/ntp-units.d 中列出的单元
static std::vector<std::string> ntp_units()
{
    std::vector<std::string> units;
    const char *dirs[] = {"/etc/systemd/ntp-units.d", "/usr/lib/systemd/ntp-units.d"};

    for (const auto &dir : dirs)
    {
        GDir *directory = g_dir_open(dir, 0, nullptr);
        if (directory == nullptr)
        {
            continue;
        }

        const gchar *entry = nullptr;
        while ((entry = g_dir_read_name(directory)) != nullptr)
        {
            gchar *path = g_build_filename(dir, entry, nullptr);
            gchar *contents = nullptr;
            if (g_file_get_contents(path, &contents, nullptr, nullptr))
            {
                std::istringstream stream(contents);
                std::string line;
                while (std::getline(stream, line))
                {
                    line = trim(line);
                    if (!line.empty() && line[0] != '#')
                    {
                        units.push_back(line);
                    }
                }
                g_free(contents);
            }
            g_free(path);
        }
        g_dir_close(directory);
    }
    return units;
}

// 与 daemon 的 ntp_is_active() 一致：active / activating 都算开启
static bool ntp_enabled()
{
    for (const auto &unit : ntp_units())
    {
        auto state = trim(run_command(("systemctl is-active " + unit).c_str()));
        if (state == "active" || state == "activating")
        {
            return true;
        }
    }
    return false;
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
    this->original_ntp_ = ntp_enabled();
}

void TimedateManagerProxy::TearDown()
{
    if (!this->timedate_proxy_)
    {
        return;
    }

    IGNORE_EXCEPTION(this->timedate_proxy_->SetTimezone_sync(this->original_timezone_));
    IGNORE_EXCEPTION(this->timedate_proxy_->SetTime_sync((gint64)now_us(), false));
    IGNORE_EXCEPTION(this->timedate_proxy_->SetNTP_sync(this->original_ntp_));
}

TEST_F(TimedateManagerProxy, SetTime)
{
    if (ntp_enabled())
    {
        ASSERT_NO_THROW(this->timedate_proxy_->SetNTP_sync(false));
    }

    guint64 before_us = now_us();
    gint64 mono_before_us = g_get_monotonic_time();
    guint64 requested_us = before_us + 5000000;

    ASSERT_NO_THROW(this->timedate_proxy_->SetTime_sync((gint64)requested_us, false));

    bool applied = false;
    for (int i = 0; i < 10; ++i)
    {
        guint64 current_us = now_us();
        guint64 diff_us = current_us > requested_us ? current_us - requested_us : requested_us - current_us;
        if (diff_us <= 200000)
        {
            applied = true;
            break;
        }
        g_usleep(100000);
    }
    // 恢复系统时间：原时间 + 期间已过去的单调时间
    guint64 elapsed_us = (guint64)(g_get_monotonic_time() - mono_before_us);
    ASSERT_NO_THROW(this->timedate_proxy_->SetTime_sync((gint64)(before_us + elapsed_us), false));

    ASSERT_TRUE(applied);
}

TEST_F(TimedateManagerProxy, SetTimezone)
{
    ASSERT_NO_THROW(this->timedate_proxy_->SetTimezone_sync("Asia/Shanghai"));
    ASSERT_EQ(localtime_zone(), "Asia/Shanghai");
}

TEST_F(TimedateManagerProxy, NTP)
{
    if (ntp_units().empty())
    {
        KIRAN_TEST_SKIP("no NTP unit found in systemd ntp-units.d");
    }

    // 按当前状态决定先切哪个方向，保证两次调用都真正触发启/停（避免 daemon 的幂等短路）
    ASSERT_NO_THROW(this->timedate_proxy_->SetNTP_sync(!this->original_ntp_));
    ASSERT_TRUE(ntp_enabled() == !this->original_ntp_);

    ASSERT_NO_THROW(this->timedate_proxy_->SetNTP_sync(this->original_ntp_));
    ASSERT_TRUE(ntp_enabled() == this->original_ntp_);
}

}  // namespace Kiran
