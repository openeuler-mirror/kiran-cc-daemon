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

#include "test/groups/test-groups-manager.h"
#include "lib/base/base.h"

#include <accounts-i.h>
#include <grp.h>
#include <groups-i.h>
#include <user_dbus_proxy.h>

#include <algorithm>
#include <string>

namespace Kiran
{
#define USER_NAME_TEST_USER1 "test_user1"
#define USER_ID_TEST_USER1 5000
#define GROUP_NAME_TEST_GROUP1 "test_group1"

void GroupsManagerProxy::CleanupUser()
{
    if (!this->accounts_proxy_)
    {
        return;
    }

    IGNORE_EXCEPTION(
        {
            Glib::DBusObjectPathString user_object_path = this->accounts_proxy_->FindUserByName_sync(USER_NAME_TEST_USER1);
            Glib::RefPtr<Kiran::SystemDaemon::Accounts::UserProxy> user_proxy =
                SystemDaemon::Accounts::UserProxy::createForBus_sync(Gio::DBus::BUS_TYPE_SYSTEM,
                                                                     Gio::DBus::PROXY_FLAGS_NONE,
                                                                     ACCOUNTS_DBUS_NAME,
                                                                     user_object_path);
            this->accounts_proxy_->DeleteUser_sync(user_proxy->uid_get(), true);
        });
}

void GroupsManagerProxy::CleanupGroup()
{
    if (!this->groups_proxy_)
    {
        return;
    }

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
    Glib::DBusObjectPathString user_object_path;
    ASSERT_NO_THROW(user_object_path = this->accounts_proxy_->CreateUser_sync(USER_NAME_TEST_USER1,
                                                                              USER_NAME_TEST_USER1,
                                                                              AccountsAccountType::ACCOUNTS_ACCOUNT_TYPE_STANDARD,
                                                                              USER_ID_TEST_USER1));
    ASSERT_FALSE(user_object_path.empty());

    Glib::DBusObjectPathString group_object_path;
    ASSERT_NO_THROW(group_object_path = this->groups_proxy_->CreateGroup_sync(GROUP_NAME_TEST_GROUP1,
                                                                              {USER_NAME_TEST_USER1}));
    ASSERT_FALSE(group_object_path.empty());

    // 通过 getgrnam 校验真实系统状态
    struct group *group = getgrnam(GROUP_NAME_TEST_GROUP1);
    ASSERT_NE(group, nullptr);
    ASSERT_EQ(std::string(group->gr_name), std::string(GROUP_NAME_TEST_GROUP1));
    gid_t group_id = group->gr_gid;

    bool user_in_group = false;
    for (char **member = group->gr_mem; member != nullptr && *member != nullptr; ++member)
    {
        if (std::string(*member) == std::string(USER_NAME_TEST_USER1))
        {
            user_in_group = true;
            break;
        }
    }
    ASSERT_TRUE(user_in_group);

    // 通过 D-Bus 校验组对象
    Glib::DBusObjectPathString found_object_path;
    ASSERT_NO_THROW(found_object_path = this->groups_proxy_->FindGroupByName_sync(GROUP_NAME_TEST_GROUP1));
    ASSERT_EQ(found_object_path, group_object_path);

    Glib::RefPtr<Kiran::SystemDaemon::Groups::GroupProxy> group_proxy =
        SystemDaemon::Groups::GroupProxy::createForBus_sync(Gio::DBus::BUS_TYPE_SYSTEM,
                                                            Gio::DBus::PROXY_FLAGS_NONE,
                                                            GROUPS_DBUS_NAME,
                                                            group_object_path);
    ASSERT_NE(!group_proxy, true);
    ASSERT_EQ(group_proxy->gid_get(), static_cast<guint64>(group_id));
    ASSERT_EQ(group_proxy->name_get(), Glib::ustring(GROUP_NAME_TEST_GROUP1));

    auto users = group_proxy->users_get();
    ASSERT_NE(std::find(users.begin(), users.end(), Glib::ustring(USER_NAME_TEST_USER1)), users.end());

    // 删除组后校验系统中已不存在
    ASSERT_NO_THROW(this->groups_proxy_->DeleteGroup_sync(group_id));
    ASSERT_EQ(getgrnam(GROUP_NAME_TEST_GROUP1), nullptr);
}

}  // namespace Kiran
