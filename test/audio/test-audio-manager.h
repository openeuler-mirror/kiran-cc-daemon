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

#include <audio_dbus_proxy.h>
#include <audio_device_dbus_proxy.h>
#include <giomm.h>
#include <gtest/gtest.h>

#include <string>

namespace Kiran
{
class AudioManagerProxy : public testing::Test
{
public:
    AudioManagerProxy(){};
    virtual ~AudioManagerProxy(){};

protected:
    virtual void SetUp() override;
    virtual void TearDown() override;

    Glib::RefPtr<SessionDaemon::AudioProxy> audio_proxy_;

    std::string original_sink_path_;
    std::string original_source_path_;
    double original_sink_volume_ = 0.0;
    double original_source_volume_ = 0.0;
    bool sink_snapshot_ = false;
    bool source_snapshot_ = false;
};
}  // namespace Kiran
