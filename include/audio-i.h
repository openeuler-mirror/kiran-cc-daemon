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

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#define AUDIO_DBUS_NAME "com.kylinsec.Kiran.SessionDaemon.Audio"
#define AUDIO_OBJECT_PATH "/com/kylinsec/Kiran/SessionDaemon/Audio"
#define AUDIO_DBUS_INTERFACE_NAME "com.kylinsec.Kiran.SessionDaemon.Audio"

#define AUDIO_SINK_OBJECT_PATH "/com/kylinsec/Kiran/SessionDaemon/Audio/Sink"
#define AUDIO_SOURCE_OBJECT_PATH "/com/kylinsec/Kiran/SessionDaemon/Audio/Source"
#define AUDIO_DEVICE_DBUS_INTERFACE_NAME "com.kylinsec.Kiran.SessionDaemon.Audio.Device"

#define AUDIO_SINK_INPUT_OBJECT_PATH "/com/kylinsec/Kiran/SessionDaemon/Audio/SinkInput"
#define AUDIO_SOURCE_OUTPUT_OBJECT_PATH "/com/kylinsec/Kiran/SessionDaemon/Audio/SourceOutput"
#define AUDIO_STREAM_DBUS_INTERFACE_NAME "com.kylinsec.Kiran.SessionDaemon.Audio.Stream"

    enum AudioState
    {
        // 还未向PulseAudio服务器发起连接
        AUDIO_STATE_IDLE,
        // 正在向PulseAudio服务器发起连接
        AUDIO_STATE_CONNECTING,
        // 数据已经准备就绪
        AUDIO_STATE_READY,
        // 连接或者数据加载失败
        AUDIO_STATE_FAILED,
        // 未知错误
        AUDIO_STATE_UNKNOWN
    };

    // 设备/流支持设置的功能，如果未指定（例如静音）则说明可读写
    enum AudioNodeState
    {
        AUDIO_NODE_STATE_NONE = 0,
        // 音量支持读取
        AUDIO_NODE_STATE_VOLUME_READABLE = 1 << 0,
        // 音量支持写入
        AUDIO_NODE_STATE_VOLUME_WRITABLE = 1 << 1,
        // 支持左右声道
        AUDIO_NODE_STATE_CAN_BALANCE = 1 << 2,
        // 支持远近声道
        AUDIO_NODE_STATE_CAN_FADE = 1 << 3,
        // 支持声音数字放大
        AUDIO_NODE_STATE_HAS_DECIBEL = 1 << 4
    };

#ifdef __cplusplus
}
#endif