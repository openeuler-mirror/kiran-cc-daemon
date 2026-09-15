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

#include "test/audio/test-audio-manager.h"
#include "test/gtest-compat.h"
#include "lib/base/base.h"

#include <audio-i.h>
#include <cmath>
#include <cstdlib>

namespace Kiran
{

void AudioManagerProxy::SetUp()
{
    const char *session_bus = getenv("DBUS_SESSION_BUS_ADDRESS");
    if (session_bus == nullptr || *session_bus == '\0')
    {
        KIRAN_TEST_SKIP("DBUS_SESSION_BUS_ADDRESS is not set");
    }

    try
    {
        this->audio_proxy_ = SessionDaemon::AudioProxy::createForBus_sync(
            Gio::DBus::BUS_TYPE_SESSION,
            Gio::DBus::PROXY_FLAGS_NONE,
            AUDIO_DBUS_NAME,
            AUDIO_OBJECT_PATH);
    }
    catch (...)
    {
        KIRAN_TEST_SKIP("session bus is not available");
    }

    auto dbus_proxy = this->audio_proxy_ ? this->audio_proxy_->dbusProxy() : Glib::RefPtr<Gio::DBus::Proxy>();
    if (!dbus_proxy || dbus_proxy->get_name_owner().empty())
    {
        // 无 GTEST_SKIP 时上面的跳过只在 SetUp 内返回，这里同时置空代理让用例体直接返回
        this->audio_proxy_.reset();
        KIRAN_TEST_SKIP("audio daemon is not running");
    }
}

void AudioManagerProxy::TearDown()
{
    if (!this->audio_proxy_)
    {
        return;
    }

    IGNORE_EXCEPTION(
        {
            if (this->sink_snapshot_ && !this->original_sink_path_.empty())
            {
                auto sink_proxy = SessionDaemon::Audio::DeviceProxy::createForBus_sync(
                    Gio::DBus::BUS_TYPE_SESSION,
                    Gio::DBus::PROXY_FLAGS_NONE,
                    AUDIO_DBUS_NAME,
                    this->original_sink_path_);
                if (sink_proxy)
                {
                    sink_proxy->SetVolume_sync(this->original_sink_volume_);
                }
            }
            if (this->source_snapshot_ && !this->original_source_path_.empty())
            {
                auto source_proxy = SessionDaemon::Audio::DeviceProxy::createForBus_sync(
                    Gio::DBus::BUS_TYPE_SESSION,
                    Gio::DBus::PROXY_FLAGS_NONE,
                    AUDIO_DBUS_NAME,
                    this->original_source_path_);
                if (source_proxy)
                {
                    source_proxy->SetVolume_sync(this->original_source_volume_);
                }
            }
        });

    this->sink_snapshot_ = false;
    this->source_snapshot_ = false;
}

TEST_F(AudioManagerProxy, SetVolume)
{
    if (!this->audio_proxy_)
    {
        return;
    }

    // 音频后端异步启动，等待其就绪后再读取默认设备
    int retries = 0;
    while (this->audio_proxy_->state_get() != AUDIO_STATE_READY && ++retries < 30)
    {
        g_usleep(100000);
    }
    ASSERT_EQ(this->audio_proxy_->state_get(), AUDIO_STATE_READY);

    std::string default_sink_path;
    ASSERT_NO_THROW(default_sink_path = this->audio_proxy_->GetDefaultSink_sync());
    ASSERT_FALSE(default_sink_path.empty());

    auto sink_proxy = SessionDaemon::Audio::DeviceProxy::createForBus_sync(
        Gio::DBus::BUS_TYPE_SESSION,
        Gio::DBus::PROXY_FLAGS_NONE,
        AUDIO_DBUS_NAME,
        default_sink_path);
    ASSERT_NE(!sink_proxy, true);

    // 音量快照须在音频后端就绪后、紧贴修改点获取（SetUp 时默认设备可能尚未就绪）；TearDown 依标志位恢复
    double original_sink_volume = 0.0;
    ASSERT_NO_THROW(original_sink_volume = sink_proxy->volume_get());
    this->original_sink_path_ = default_sink_path;
    this->original_sink_volume_ = original_sink_volume;
    this->sink_snapshot_ = true;

    ASSERT_NO_THROW(sink_proxy->SetVolume_sync(0.5));
    auto sink_proxy_live = SessionDaemon::Audio::DeviceProxy::createForBus_sync(
        Gio::DBus::BUS_TYPE_SESSION,
        Gio::DBus::PROXY_FLAGS_NONE,
        AUDIO_DBUS_NAME,
        default_sink_path);
    ASSERT_NE(!sink_proxy_live, true);
    double sink_volume = sink_proxy_live->volume_get();
    ASSERT_TRUE(std::fabs(sink_volume - 0.5) < 0.001);

    std::string default_source_path;
    ASSERT_NO_THROW(default_source_path = this->audio_proxy_->GetDefaultSource_sync());
    ASSERT_FALSE(default_source_path.empty());

    auto source_proxy = SessionDaemon::Audio::DeviceProxy::createForBus_sync(
        Gio::DBus::BUS_TYPE_SESSION,
        Gio::DBus::PROXY_FLAGS_NONE,
        AUDIO_DBUS_NAME,
        default_source_path);
    ASSERT_NE(!source_proxy, true);

    double original_source_volume = 0.0;
    ASSERT_NO_THROW(original_source_volume = source_proxy->volume_get());
    this->original_source_path_ = default_source_path;
    this->original_source_volume_ = original_source_volume;
    this->source_snapshot_ = true;

    ASSERT_NO_THROW(source_proxy->SetVolume_sync(0.5));
    auto source_proxy_live = SessionDaemon::Audio::DeviceProxy::createForBus_sync(
        Gio::DBus::BUS_TYPE_SESSION,
        Gio::DBus::PROXY_FLAGS_NONE,
        AUDIO_DBUS_NAME,
        default_source_path);
    ASSERT_NE(!source_proxy_live, true);
    double source_volume = source_proxy_live->volume_get();
    ASSERT_TRUE(std::fabs(source_volume - 0.5) < 0.001);
}

}  // namespace Kiran
