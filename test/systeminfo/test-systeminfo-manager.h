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

#include <systeminfo_dbus_proxy.h>
#include <giomm.h>
#include <gtest/gtest.h>

#include <string>

namespace Kiran
{
class SystemInfoManagerProxy : public testing::Test
{
public:
    SystemInfoManagerProxy(){};
    virtual ~SystemInfoManagerProxy(){};

protected:
    virtual void SetUp() override;
    virtual void TearDown() override;

    Glib::RefPtr<SystemDaemon::SystemInfoProxy> systeminfo_proxy_;
    std::string original_hostname_;
    bool snapshot_valid_ = false;
};
}  // namespace Kiran
