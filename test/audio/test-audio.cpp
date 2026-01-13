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
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
 */

#include "test-audio.h"
#include <QDBusConnection>
#include "audio-i.h"
#include "plugins/audio/audio-plugin.h"
#include "test/audio/audio_device_interface.h"
#include "test/audio/audio_interface.h"

namespace Kiran
{

void TestAudio::initTestCase()
{
    qputenv("GSETTINGS_BACKEND", "memory");

    m_plugin = new AudioPlugin();
    m_plugin->activate();
    m_audioProxy = new AudioProxy(AUDIO_DBUS_NAME,
                                  AUDIO_OBJECT_PATH,
                                  QDBusConnection::sessionBus(),
                                  this);
}

void TestAudio::cleanupTestCase()
{
    m_plugin->deactivate();
    delete m_plugin;
    m_plugin = nullptr;
}

void TestAudio::testSetVolume()
{
    // 等待声音后端初始化完成
    int i = 0;
    while (m_audioProxy->state() != AudioState::AUDIO_STATE_READY && ++i < 30)
    {
        QTest::qWait(100);
    }

    // 设置扬声器音量
    auto defaultSink = m_audioProxy->GetDefaultSink();
    auto m_sinkProxy = new AudioDeviceProxy(AUDIO_DBUS_NAME, defaultSink.value(), QDBusConnection::sessionBus(), this);
    m_sinkProxy->SetVolume(0.5).waitForFinished();
    QVERIFY(qFabs(m_sinkProxy->volume() - 0.5) < 0.001);

    // 设置麦克风音量
    auto defaultSource = m_audioProxy->GetDefaultSource();
    auto m_sourceProxy = new AudioDeviceProxy(AUDIO_DBUS_NAME, defaultSource.value(), QDBusConnection::sessionBus(), this);
    m_sourceProxy->SetVolume(0.5).waitForFinished();
    QVERIFY(qFabs(m_sourceProxy->volume() - 0.5) < 0.001);
}

}  // namespace Kiran

QTEST_MAIN(Kiran::TestAudio)

#include "test-audio.moc"