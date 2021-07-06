/**
 * @Copyright (C) 2020 ~ 2021 KylinSec Co., Ltd. 
 *
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; If not, see <http: //www.gnu.org/licenses/>. 
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