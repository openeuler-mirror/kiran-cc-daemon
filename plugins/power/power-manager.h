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

class QGSettings;

namespace Kiran
{
class PowerWrapperManager;
class PowerBacklight;
class PowerUPower;
class PowerProfiles;
class PowerBacklightPercentage;

class PowerManager : public QObject,
                     protected QDBusContext
{
    Q_OBJECT

public:
    Q_PROPERTY(int ActiveProfile READ getActiveProfile WRITE setActiveProfile)
    Q_PROPERTY(bool ChargeLowDimmedEnabled READ getChargeLowDimmedEnabled WRITE setChargeLowDimmedEnabled)
    Q_PROPERTY(bool ChargeLowSaverEnabled READ getChargeLowSaverEnabled WRITE setChargeLowSaverEnabled)
    Q_PROPERTY(bool DisplayIdleDimmedEnabled READ getDisplayIdleDimmedEnabled WRITE setDisplayIdleDimmedEnabled)
    Q_PROPERTY(bool LidIsPresent READ getLidIsPresent WRITE setLidIsPresent)
    Q_PROPERTY(bool OnBattery READ getOnBattery WRITE setOnBattery)

    int getActiveProfile() const;
    bool getChargeLowDimmedEnabled() const;
    bool getChargeLowSaverEnabled() const;
    bool getDisplayIdleDimmedEnabled() const;
    bool getLidIsPresent() const;
    // 系统是否在使用电池供电
    bool getOnBattery() const;

    void setActiveProfile(int activeProfile);
    void setChargeLowDimmedEnabled(bool enabled);
    void setChargeLowSaverEnabled(bool enabled);
    void setDisplayIdleDimmedEnabled(bool enabled);
    void setLidIsPresent(bool lidIsPresent);
    void setOnBattery(bool onBattery);

public Q_SLOTS:
    // 电量过低时显示器是否变暗
    void EnableChargeLowDimmed(bool enabled);
    // 电量过低时是否进入节能模式
    void EnableChargeLowSaver(bool enabled);
    // 空闲时显示器是否变暗
    void EnableDisplayIdleDimmed(bool enabled);
    // 获取设备的亮度百分比，如果设备不支持则返回-1
    int GetBrightness(int device);
    // 获取某个事件触发时的动作
    int GetEventAction(int event);
    // 获取计算机空闲时的处理动作
    QString GetIdleAction(int device, int supply);
    /**
     * @brief 设置指定设备的亮度百分比。
     * @param {device} 可以设置为显示器或者键盘
     * @param {brightness_percentage} 亮度百分比，范围为[0, 100]
     * @return 如果参数错误或者不支持设置亮度则返回错误
     */
    void SetBrightness(int device, int brightnessPercentage);
    /**
     * @brief 设置某个事件触发时的动作
     * @param {event} 事件类型，参考PowerEvent的定义
     * @param {action} 触发的动作，参考PowerAction的定义
     * @return 参数错误或者写入gsetetings失败则返回错误，否则返回成功
     */
    void SetEventAction(int event, int action);
    /**
     * @brief 设置计算机空闲时，在不同供电情况下(supply指定)，不同设备(device指定)的空闲超时(idle_timeout指定)处理动作(action指定)
     * @param {device} 可以指定为计算机或者背光设备，参考PowerDeviceType的定义
     * @param {supply} 系统供电方式，参考PowerSupplyMode的定义
     * @param {idleTimeout} 空闲超时时间，空闲时间超过该时间则触发动作
     * @param {action} 触发的动作，参考PowerAction的定义
     * @return {} 参数错误或者写入gsetetings失败则返回错误，否则返回成功
     */
    void SetIdleAction(int device, int supply, int idleTimeout, int action);
    // 切换电源模式
    void SwitchProfile(int mode);

Q_SIGNALS:  // SIGNALS
    void ActiveProfileChanged(int active_profile);
    void BrightnessChanged(int device);
    void EventActionChanged(int event);
    void IdleActionChanged(int device, int supply);

public:
    PowerManager(PowerWrapperManager *wrapper_manager, PowerBacklight *backlight);
    virtual ~PowerManager();

    static PowerManager *getInstance() { return m_instance; };

    static void globalInit(PowerWrapperManager *wrapper_manager, PowerBacklight *backlight);

    static void globalDeinit() { delete m_instance; };

private:
    void init();

    void processSettingsChanged(const QString &key);
    void processBrightnessChanged(PowerBacklightPercentage *backlightDevice, int32_t brightnessValue);
    void processActiveProfileChanged(int32_t profileMode);

private:
    static PowerManager *m_instance;

    PowerWrapperManager *m_wrapperManager;
    PowerBacklight *m_backlight;
    QSharedPointer<PowerUPower> m_upowerClient;
    QSharedPointer<PowerProfiles> m_profiles;
    QGSettings *m_powerSettings;
};
}  // namespace Kiran