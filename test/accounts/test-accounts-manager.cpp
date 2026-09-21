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

#include <accounts-i.h>
#include <pwd.h>
#include <user_dbus_proxy.h>

namespace Kiran
{
#define USRE_NAME_TEST001 "test001"
#define USRE_ID_TEST001 5000

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

void AccountsManagerProxy::CleanupUser()
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
            this->accounts_proxy_->FindUserByName_sync(USRE_NAME_TEST001);
            this->accounts_proxy_->DeleteUser_sync(USRE_ID_TEST001, true);
        });
    wait_user_removed(this->accounts_proxy_, USRE_NAME_TEST001, 1000);
}

void AccountsManagerProxy::SetUp()
{
    this->accounts_proxy_ = SystemDaemon::AccountsProxy::createForBus_sync(Gio::DBus::BUS_TYPE_SYSTEM,
                                                                           Gio::DBus::PROXY_FLAGS_NONE,
                                                                           ACCOUNTS_DBUS_NAME,
                                                                           ACCOUNTS_OBJECT_PATH);

    ASSERT_NE(!this->accounts_proxy_, true);

    this->CleanupUser();
}

void AccountsManagerProxy::TearDown()
{
    this->CleanupUser();
}

TEST_F(AccountsManagerProxy, CreateUser)
{
    Glib::DBusObjectPathString user_object_path;
    Glib::RefPtr<Kiran::SystemDaemon::Accounts::UserProxy> user_proxy;

    // 创建帐户 test001
    ASSERT_NO_THROW(user_object_path = this->accounts_proxy_->CreateUser_sync(USRE_NAME_TEST001,
                                                                              USRE_NAME_TEST001,
                                                                              AccountsAccountType::ACCOUNTS_ACCOUNT_TYPE_STANDARD,
                                                                              USRE_ID_TEST001));

    ASSERT_NO_THROW(user_proxy = SystemDaemon::Accounts::UserProxy::createForBus_sync(Gio::DBus::BUS_TYPE_SYSTEM,
                                                                                      Gio::DBus::PROXY_FLAGS_NONE,
                                                                                      ACCOUNTS_DBUS_NAME,
                                                                                      user_object_path));

    ASSERT_NE(!user_proxy, true);
    ASSERT_EQ(USRE_NAME_TEST001, user_proxy->user_name_get());

    // 调用 getpwnam 校验帐户已创建
    struct passwd *pw = getpwnam(USRE_NAME_TEST001);
    ASSERT_NE(pw, nullptr);
    ASSERT_EQ(std::string(pw->pw_name), USRE_NAME_TEST001);

    // 删除帐户 test001，并校验已从系统中删除
    ASSERT_NO_THROW(this->accounts_proxy_->DeleteUser_sync(USRE_ID_TEST001, true));
    ASSERT_EQ(getpwnam(USRE_NAME_TEST001), nullptr);
}
}  // namespace Kiran
