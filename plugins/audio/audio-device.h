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

class DeviceAdaptor;

namespace Kiran
{
class PulseDevice;

class AudioDevice : public QObject,
                    protected QDBusContext
{
    Q_OBJECT

    Q_PROPERTY(QString active_port READ getActivePort WRITE setActivePort)
    Q_PROPERTY(double balance READ getBalance WRITE setBalance)
    Q_PROPERTY(double base_volume READ getBaseVolume)
    Q_PROPERTY(uint card_index READ getCardIndex)
    Q_PROPERTY(double fade READ getFade WRITE setFade)
    Q_PROPERTY(uint index READ getIndex)
    Q_PROPERTY(bool mute READ getMute WRITE setMute)
    Q_PROPERTY(QString name READ getName)
    Q_PROPERTY(uint state READ getState)
    Q_PROPERTY(double volume READ getVolume WRITE setVolume)

public:
    AudioDevice(QSharedPointer<PulseDevice> device);
    virtual ~AudioDevice();

    bool init(const QString &objectPathPrefix);

    QString getObjectPath() { return m_objectPath; };

public:
    QString getActivePort() const { return m_activePort; };
    double getBalance() const { return m_balance; };
    double getBaseVolume() const;
    uint getCardIndex() const;
    double getFade() const { return m_fade; };
    uint getIndex() const;
    bool getMute() const { return m_mute; };
    QString getName() const;
    uint getState() const;
    double getVolume() const { return m_volume; };

    void setActivePort(const QString &activePort);
    void setBalance(double balance);
    void setFade(double fade);
    void setMute(bool mute);
    void setVolume(double volume);

public Q_SLOTS:
    // 获取所有绑定端口
    QString
    GetPorts();
    // 获取属性
    QString GetProperty(const QString &key);
    // 设置正在使用的端口
    void SetActivePort(const QString &name);
    // 设置左声道和右声道的平衡
    void SetBalance(double balance);
    // 设置远声道和近声道的平衡
    void SetFade(double fade);
    // 设置静音
    void SetMute(bool mute);
    // 设置音量
    void SetVolume(double volume);

private:
    bool dbusRegister();
    void dbusUnregister();

    void processNodeInfoChanged(int32_t field);

private:
    DeviceAdaptor *m_adaptor;
    QSharedPointer<PulseDevice> m_device;
    QString m_objectPath;
    bool m_mute;
    double m_volume;
    double m_balance;
    double m_fade;
    QString m_activePort;
};

}  // namespace Kiran
