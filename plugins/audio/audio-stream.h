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

#include <QDBusContext>
#include <QSharedPointer>

class StreamAdaptor;

namespace Kiran
{
class PulseStream;

class AudioStream : public QObject,
                    protected QDBusContext
{
    Q_OBJECT

    Q_PROPERTY(uint index READ getIndex)
    Q_PROPERTY(bool mute READ getMute WRITE setMute)
    Q_PROPERTY(QString name READ getName)
    Q_PROPERTY(uint state READ getState)
    Q_PROPERTY(double volume READ getVolume WRITE setVolume)

public:
    AudioStream(QSharedPointer<PulseStream> stream);
    virtual ~AudioStream();

    bool init(const QString &objectPathPrefix);

    QString getObjectPath() { return m_objectPath; };

public:
    uint getIndex() const;
    bool getMute() const { return m_mute; };
    QString getName() const;
    uint getState() const;
    double getVolume() const { return m_volume; };

    void setMute(bool mute);
    void setVolume(double volume);

public Q_SLOTS:
    // 获取属性
    QString GetProperty(const QString &key);
    // 设置静音
    void SetMute(bool mute);
    // 设置音量
    void SetVolume(double volume);

private:
    bool dbusRegister();
    void dbusUnregister();

    void processNodeInfoChanged(int32_t field);

private:
    StreamAdaptor *m_adaptor;
    QSharedPointer<PulseStream> m_stream;
    QString m_objectPath;
    bool m_mute;
    double m_volume;
};
}  // namespace Kiran