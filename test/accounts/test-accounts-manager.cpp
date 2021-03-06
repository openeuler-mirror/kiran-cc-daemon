/**
 * Copyright (c) 2020 ~ 2021 KylinSec Co., Ltd. 
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

#include "test/accounts/test-accounts-manager.h"
#include "lib/base/base.h"

#define ACCOUNTS_NEW_INTERFACE
#include <accounts_i.h>
#include <user_dbus_proxy.h>

namespace Kiran
{
#define USRE_NAME_TEST001 "test001"

void AccountsManagerProxy::SetUp()
{
    this->accounts_proxy_ = SystemDaemon::AccountsProxy::createForBus_sync(Gio::DBus::BUS_TYPE_SYSTEM,
                                                                           Gio::DBus::PROXY_FLAGS_NONE,
                                                                           ACCOUNTS_DBUS_NAME,
                                                                           ACCOUNTS_OBJECT_PATH);

    ASSERT_NE(!this->accounts_proxy_, true);
}

void AccountsManagerProxy::TearDown()
{
}

TEST_F(AccountsManagerProxy, CreateUser)
{
    Glib::DBusObjectPathString user_object_path;
    Glib::RefPtr<Kiran::SystemDaemon::Accounts::UserProxy> user_proxy;

    // 删除test001用户
    IGNORE_EXCEPTION({
        user_object_path = this->accounts_proxy_->FindUserByName_sync(USRE_NAME_TEST001);
        user_proxy = SystemDaemon::Accounts::UserProxy::createForBus_sync(Gio::DBus::BUS_TYPE_SYSTEM,
                                                                          Gio::DBus::PROXY_FLAGS_NONE,
                                                                          ACCOUNTS_DBUS_NAME,
                                                                          user_object_path);
        this->accounts_proxy_->DeleteUser_sync(user_proxy->uid_get(), true);
    });

    // 创建test001用户
    ASSERT_NO_THROW(user_object_path = this->accounts_proxy_->CreateUser_sync(USRE_NAME_TEST001,
                                                                              USRE_NAME_TEST001,
                                                                              AccountsAccountType::ACCOUNTS_ACCOUNT_TYPE_STANDARD,
                                                                              -1));

    ASSERT_NO_THROW(user_proxy = SystemDaemon::Accounts::UserProxy::createForBus_sync(Gio::DBus::BUS_TYPE_SYSTEM,
                                                                                      Gio::DBus::PROXY_FLAGS_NONE,
                                                                                      ACCOUNTS_DBUS_NAME,
                                                                                      user_object_path));

    ASSERT_NE(!user_proxy, true);
    ASSERT_EQ(USRE_NAME_TEST001, user_proxy->user_name_get());
}
}  // namespace Kiran