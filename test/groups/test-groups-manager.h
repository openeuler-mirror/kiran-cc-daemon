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

#pragma once

#include <accounts_dbus_proxy.h>
#include <giomm.h>
#include <group_dbus_proxy.h>
#include <groups_dbus_proxy.h>
#include <gtest/gtest.h>

namespace Kiran
{
class GroupsManagerProxy : public testing::Test
{
public:
    GroupsManagerProxy(){};
    virtual ~GroupsManagerProxy(){};

protected:
    // Sets up the test fixture.
    virtual void SetUp() override;

    // Tears down the test fixture.
    virtual void TearDown() override;

    void CleanupGroup();
    void CleanupUser();

    Glib::RefPtr<SystemDaemon::GroupsProxy> groups_proxy_;
    Glib::RefPtr<SystemDaemon::AccountsProxy> accounts_proxy_;
};
}  // namespace Kiran
