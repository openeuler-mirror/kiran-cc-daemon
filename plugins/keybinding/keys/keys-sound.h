/**
 * Copyright (c) 2024 ~ 2025 KylinSec Co., Ltd.
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

#pragma once

#include "keys-component.h"

class AudioProxy;
class AudioDeviceProxy;
class QDBusServiceWatcher;

namespace Kiran
{

class KeysSound : public KeysComponent
{
    Q_OBJECT

public:
    KeysSound();
    virtual ~KeysSound(){};

    virtual void init();

private:
    void initAudioProxy();
    // 设备禁音
    void muteDevice(AudioDeviceProxy *deviceProxy);
    // 降低设备音量
    void downDeviceVolume(AudioDeviceProxy *deviceProxy);
    // 提升设备音量
    void upDeviceVolume(AudioDeviceProxy *deviceProxy);
    // 设置设备音量
    void setDeviceVolume(AudioDeviceProxy *deviceProxy, double volume);

    void updateAudioDevice();
    void updateAudioSinkDevice();
    void updateAudioSourceDevice();

private:
    virtual void triggerShortCut(const QString &name);

private:
    AudioProxy *m_audioProxy;
    AudioDeviceProxy *m_audioSinkDeviceProxy;
    AudioDeviceProxy *m_audioSourceDeviceProxy;
    QDBusServiceWatcher *m_audioServiceWatcher;
};

}  // namespace Kiran
