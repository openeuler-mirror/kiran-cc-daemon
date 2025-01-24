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

#include <pulse/introspect.h>
#include <QMap>
#include <QObject>
#include "audio-i.h"

/*
mute ------ base ------ norm ------ norm * n (max)
mute : 静音，一般对应0分贝
base : 依赖于硬件设备
norm : 硬件设备支持的最大音量
norm * n (max) : 超过硬件的最大音量，PA(PulseAudio)会进行数字放大，仅仅适用于已经设置PA_SINK_DECIBEL_VOLUME/PA_SOURCE_DECIBEL_VOLUME的设备
*/

namespace Kiran
{
class PulseCard;

// PulseStream字段变化信号的标记
enum PulseNodeField
{
    PULSE_NODE_FIELD_MUTE = 0,
    PULSE_NODE_FIELD_VOLUME,
    PULSE_NODE_FIELD_BALANCE,
    PULSE_NODE_FIELD_FADE,
};

struct PulseNodeInfo
{
public:
    uint32_t index;
    QString name;
    // 控制声道
    pa_channel_map channelMap;
    // 声音
    pa_cvolume cvolume;
    // 静音
    int32_t mute;
    // 基本音量，一般又硬件决定
    pa_volume_t baseVolume;
    // 属性
    pa_proplist *proplist;
};

class PulseNode : public QObject
{
    Q_OBJECT

public:
    PulseNode(const PulseNodeInfo &nodeInfo);

    virtual ~PulseNode() {};

    uint32_t getIndex() { return m_index; };
    const QString &getName() { return m_name; };

    // 获取静音
    bool getMute() { return m_mute; };
    // 获取音量
    uint32_t getVolume() { return m_volume; };
    // 获取左声道和右声道的平衡
    float getBalance() { return m_balance; };
    // 获取前声道和后声道的平衡
    float getFade() { return m_fade; }
    //
    AudioNodeState getFlags() { return m_flags; };

    // 设置静音
    bool setMute(bool mute);
    // 设置音量
    bool setVolume(uint32_t volume);
    // 设置左右平衡，balance的范围为[-1, 1]
    bool setBalance(float balance);
    // 设置前后平衡，范围为[-1, 1]
    bool setFade(float fade);

    // 获取最小音量
    uint32_t getMinVolume() { return uint32_t(PA_VOLUME_MUTED); };
    // 获取最大音量
    uint32_t getMaxVolume();
    // 获取正常音量
    uint32_t getNormalVolume() { return uint32_t(PA_VOLUME_NORM); };
    // base音量
    uint32_t getBaseVolume();

    // 获取属性
    QString getProperty(const QString &key) { return m_attrs.value(key); };

Q_SIGNALS:
    void nodeInfoChanged(int32_t field);

protected:
    void update(const pa_channel_map &channel_map,
                const pa_cvolume &cvolume,
                int32_t mute,
                pa_volume_t base_volume);

    virtual bool setMute(int32_t mute);
    virtual bool setCvolume(const pa_cvolume &cvolume);
    // 构造函数中需要调用，不能设为虚函数
    void updateFlags();

private:
    void updateMute(bool mute);
    void updateVolume(uint32_t volume);
    void updateBanlance(float balance);
    void updateFade(float fade);
    bool updateCvolume(const pa_cvolume &cvolume);

protected:
    AudioNodeState m_flags;

private:
    uint32_t m_index;
    QString m_name;
    pa_channel_map m_channelMap;
    pa_cvolume m_cvolume;
    pa_volume_t m_baseVolume;

    // 静音
    bool m_mute;
    // 音量
    uint32_t m_volume;
    // 左右声道的平衡
    float m_balance;
    // 前后声道的平衡
    float m_fade;

    // 属性信息
    QMap<QString, QString> m_attrs;
};
}  // namespace Kiran