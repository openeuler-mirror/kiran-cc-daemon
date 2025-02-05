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

class QGSettings;
class MouseAdaptor;

namespace Kiran
{
class InputBackend;

class MouseManager : public QObject,
                     protected QDBusContext
{
    Q_OBJECT

    Q_PROPERTY(bool left_handed READ getLeftHanded WRITE setLeftHanded)
    Q_PROPERTY(bool middle_emulation_enabled READ getMiddleEmulationEnabled WRITE setMiddleEmulationEnabled)
    Q_PROPERTY(double motion_acceleration READ getMotionAcceleration WRITE setMotionAcceleration)
    Q_PROPERTY(bool natural_scroll READ getNaturalScroll WRITE setNaturalScroll)

public:
    MouseManager();
    virtual ~MouseManager();

    static MouseManager *get_instance() { return m_instance; };
    static void globalInit();
    static void globalDeinit() { delete m_instance; };

public:
    // 左手模式，会对鼠标左键和右键的功能进行互换
    bool getLeftHanded() const { return m_leftHanded; };
    // 开启鼠标滚动键仿真效果，通过同时点击鼠标左键和右键来触发滚轮点击事件
    bool getMiddleEmulationEnabled() const { return m_middleEmulationEnabled; };
    // 移动加速，范围为[-1,1]
    double getMotionAcceleration() const { return m_motionAcceleration; };
    // 自然滚动，如果设置为false（默认值），那么鼠标滚轮向下时页面滚动也向下；如果设置为true，那么鼠标滚轮向下时页面滚动则向上。
    bool getNaturalScroll() const { return m_naturalScroll; };

    void setLeftHanded(bool leftHandled);
    void setMiddleEmulationEnabled(bool middleEmulationEnabled);
    void setMotionAcceleration(double motionAcceleration);
    void setNaturalScroll(bool naturalScroll);

public Q_SLOTS:  // METHODS
    void Reset();

private:
    void init();
    void loadFromSettings();

    void processSettingsChanged(const QString &key);

    void setAllPropsToDevices();
    void setLeftHandedToDevices();
    void setMotionAccelerationToDevices();
    void setMiddleEmulationEnabledToDevices();
    void setNaturalScrollToDevices();
    void setPropToDevices(const QString &name, float value);
    void setPropToDevices(const QString &name, const QVector<bool> &values);

private:
    static MouseManager *m_instance;
    MouseAdaptor *m_adaptor;
    QGSettings *m_mouseSettings;
    InputBackend *m_inputBackend;

    bool m_leftHanded;
    double m_motionAcceleration;
    bool m_middleEmulationEnabled;
    bool m_naturalScroll;
};
}  // namespace Kiran