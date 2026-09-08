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

#include <appearance_dbus_proxy.h>
#include <giomm.h>
#include <gtest/gtest.h>

#include <map>
#include <string>

namespace Kiran
{
class AppearanceManagerProxy : public testing::Test
{
public:
    AppearanceManagerProxy(){};
    virtual ~AppearanceManagerProxy(){};

protected:
    virtual void SetUp() override;
    virtual void TearDown() override;

    Glib::RefPtr<SessionDaemon::AppearanceProxy> appearance_proxy_;

    std::map<int, std::string> original_themes_;
    std::map<int, std::string> original_fonts_;
    std::string original_desktop_background_;
    std::string original_lockscreen_background_;
    bool snapshot_valid_ = false;
};
}  // namespace Kiran
