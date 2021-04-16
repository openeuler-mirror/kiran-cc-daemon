/**
 * @file          /kiran-cc-daemon/include/audio_i.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
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

    enum PulseState
    {
        // 还未向PulseAudio服务器发起连接
        PULSE_STATE_IDLE,
        // 正在向PulseAudio服务器发起连接
        PULSE_STATE_CONNECTING,
        // 数据已经准备就绪
        PULSE_STATE_READY,
        // 连接或者数据加载失败
        PULSE_STATE_FAILED,
        // 未知错误
        PULSE_STATE_UNKNOWN
    };

    // 设备/流支持设置的功能，如果未指定（例如静音）则说明可读写
    enum PulseNodeState
    {
        PULSE_NODE_STATE_NONE = 0,
        // 音量支持读取
        PULSE_NODE_STATE_VOLUME_READABLE = 1 << 0,
        // 音量支持写入
        PULSE_NODE_STATE_VOLUME_WRITABLE = 1 << 1,
        // 支持左右声道
        PULSE_NODE_STATE_CAN_BALANCE = 1 << 2,
        // 支持远近声道
        PULSE_NODE_STATE_CAN_FADE = 1 << 3,
        // 支持声音数字放大
        PULSE_NODE_STATE_HAS_DECIBEL = 1 << 4
    };

#ifdef __cplusplus
}
#endif