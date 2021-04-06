/**
 * @file          /kiran-cc-daemon/test/accounts/test_accounts_manager.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#include <accounts_dbus_proxy.h>
#include <giomm.h>
#include <gtest/gtest.h>

namespace Kiran
{
class AccountsManagerProxy : public testing::Test
{
public:
    AccountsManagerProxy(){};
    virtual ~AccountsManagerProxy(){};

protected:
    // Sets up the test fixture.
    virtual void SetUp() override;

    // Tears down the test fixture.
    virtual void TearDown() override;

    Glib::RefPtr<SystemDaemon::AccountsProxy> accounts_proxy_;
};
}  // namespace Kiran