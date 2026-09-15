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

#include "test/groups/test-groups-manager.h"
#include "test/gtest-compat.h"
#include "lib/base/base.h"

#include <accounts-i.h>
#include <grp.h>
#include <groups-i.h>
#include <pwd.h>

#include <string>

namespace Kiran
{
#define USER_NAME_TEST_USER1 "test_user1"
// 使用独立 uid：daemon 按 uid 查找用户，与其他用例共用 uid 会因缓存滞后删错用户、留下残留
#define USER_ID_TEST_USER1 5001
#define GROUP_NAME_TEST_GROUP1 "test_group1"

// 组成员以 /etc/group 为准，避免读到 daemon 中滞后的缓存
static bool group_has_user(const char *group_name, const char *user_name)
{
    struct group *group = getgrnam(group_name);
    if (group == nullptr)
    {
        return false;
    }
    for (char **member = group->gr_mem; member != nullptr && *member != nullptr; ++member)
    {
        if (std::string(*member) == user_name)
        {
            return true;
        }
    }
    return false;
}

// daemon 的缓存在删除后经文件监控异步刷新，等到守护进程与系统中都不再存在再继续
static bool wait_user_removed(const Glib::RefPtr<SystemDaemon::AccountsProxy> &accounts_proxy,
                              const std::string &user_name,
                              int timeout_ms)
{
    for (int waited = 0; waited <= timeout_ms; waited += 100)
    {
        bool known_by_daemon = true;
        try
        {
            accounts_proxy->FindUserByName_sync(user_name);
        }
        catch (const Glib::Error &)
        {
            // 查不到会抛 Glib::Error，即 daemon 已不再持有该用户
            known_by_daemon = false;
        }

        if (!known_by_daemon && getpwnam(user_name.c_str()) == nullptr)
        {
            return true;
        }
        g_usleep(100000);
    }
    return false;
}

static bool wait_group_removed(const Glib::RefPtr<SystemDaemon::GroupsProxy> &groups_proxy,
                               const std::string &group_name,
                               int timeout_ms)
{
    for (int waited = 0; waited <= timeout_ms; waited += 100)
    {
        bool known_by_daemon = true;
        try
        {
            groups_proxy->FindGroupByName_sync(group_name);
        }
        catch (const Glib::Error &)
        {
            // 查不到会抛 Glib::Error，即 daemon 已不再持有该组
            known_by_daemon = false;
        }

        if (!known_by_daemon && getgrnam(group_name.c_str()) == nullptr)
        {
            return true;
        }
        g_usleep(100000);
    }
    return false;
}

void GroupsManagerProxy::CleanupUser()
{
    if (!this->accounts_proxy_)
    {
        return;
    }

    // 清理只用于复位环境：daemon 缓存里仍留有该用户时才删除，删除后等其真正消失。
    // 用户不存在时 FindUserByName_sync 会抛 Glib::Error，异常直接从 IGNORE_EXCEPTION 的 try 块跳出，
    // 后面的 DeleteUser_sync 不会执行，因此不会去删一个不存在的用户、也不会多报一个删除错误
    IGNORE_EXCEPTION(
        {
            this->accounts_proxy_->FindUserByName_sync(USER_NAME_TEST_USER1);
            this->accounts_proxy_->DeleteUser_sync(USER_ID_TEST_USER1, true);
        });
    wait_user_removed(this->accounts_proxy_, USER_NAME_TEST_USER1, 1000);
}

void GroupsManagerProxy::CleanupGroup()
{
    if (!this->groups_proxy_)
    {
        return;
    }

    // 清理只用于复位环境：按 daemon 缓存里的组对象删除，等其真正消失。
    // 组不存在时 FindGroupByName_sync 会抛 Glib::Error，异常跳出 try 块，
    // 后面的建 GroupProxy 与 DeleteGroup_sync 都不会执行
    IGNORE_EXCEPTION(
        {
            Glib::DBusObjectPathString group_object_path = this->groups_proxy_->FindGroupByName_sync(GROUP_NAME_TEST_GROUP1);
            Glib::RefPtr<Kiran::SystemDaemon::Groups::GroupProxy> group_proxy =
                SystemDaemon::Groups::GroupProxy::createForBus_sync(Gio::DBus::BUS_TYPE_SYSTEM,
                                                                    Gio::DBus::PROXY_FLAGS_NONE,
                                                                    GROUPS_DBUS_NAME,
                                                                    group_object_path);
            this->groups_proxy_->DeleteGroup_sync(group_proxy->gid_get());
        });
    wait_group_removed(this->groups_proxy_, GROUP_NAME_TEST_GROUP1, 1000);
}

void GroupsManagerProxy::SetUp()
{
    this->groups_proxy_ = SystemDaemon::GroupsProxy::createForBus_sync(Gio::DBus::BUS_TYPE_SYSTEM,
                                                                       Gio::DBus::PROXY_FLAGS_NONE,
                                                                       GROUPS_DBUS_NAME,
                                                                       GROUPS_OBJECT_PATH);
    ASSERT_NE(!this->groups_proxy_, true);

    this->accounts_proxy_ = SystemDaemon::AccountsProxy::createForBus_sync(Gio::DBus::BUS_TYPE_SYSTEM,
                                                                           Gio::DBus::PROXY_FLAGS_NONE,
                                                                           ACCOUNTS_DBUS_NAME,
                                                                           ACCOUNTS_OBJECT_PATH);
    ASSERT_NE(!this->accounts_proxy_, true);

    this->CleanupGroup();
    this->CleanupUser();
}

void GroupsManagerProxy::TearDown()
{
    this->CleanupGroup();
    this->CleanupUser();
}

TEST_F(GroupsManagerProxy, CreateGroup)
{
    // 建用户：CreateGroup 的成员需要该帐户已存在
    ASSERT_NO_THROW(this->accounts_proxy_->CreateUser_sync(USER_NAME_TEST_USER1,
                                                           USER_NAME_TEST_USER1,
                                                           AccountsAccountType::ACCOUNTS_ACCOUNT_TYPE_STANDARD,
                                                           USER_ID_TEST_USER1));
    ASSERT_NE(getpwnam(USER_NAME_TEST_USER1), nullptr);

    // 建组并带上成员
    Glib::DBusObjectPathString group_object_path;
    ASSERT_NO_THROW(group_object_path = this->groups_proxy_->CreateGroup_sync(GROUP_NAME_TEST_GROUP1, {USER_NAME_TEST_USER1}));
    ASSERT_FALSE(group_object_path.empty());

    // 真实系统状态：组已创建、组名正确且成员在组内
    struct group *group = getgrnam(GROUP_NAME_TEST_GROUP1);
    ASSERT_NE(group, nullptr);
    ASSERT_EQ(std::string(group->gr_name), std::string(GROUP_NAME_TEST_GROUP1));
    ASSERT_TRUE(group_has_user(GROUP_NAME_TEST_GROUP1, USER_NAME_TEST_USER1));

    // 删组与用户，并校验两者都已从系统中删除
    ASSERT_NO_THROW(this->groups_proxy_->DeleteGroup_sync(group->gr_gid));
    ASSERT_NO_THROW(this->accounts_proxy_->DeleteUser_sync(USER_ID_TEST_USER1, true));
    ASSERT_EQ(getgrnam(GROUP_NAME_TEST_GROUP1), nullptr);
    ASSERT_EQ(getpwnam(USER_NAME_TEST_USER1), nullptr);
}

}  // namespace Kiran
