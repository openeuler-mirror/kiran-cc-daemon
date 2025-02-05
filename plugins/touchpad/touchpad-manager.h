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
class TouchPadAdaptor;

namespace Kiran
{
class InputBackend;

class TouchPadManager : public QObject,
                        protected QDBusContext
{
    Q_OBJECT

    Q_PROPERTY(int click_method READ getClickMethod WRITE setClickMethod)
    Q_PROPERTY(bool click_method_support READ getClickMethodSupport WRITE setClickMethodSupport)
    Q_PROPERTY(bool disable_while_typing READ getDisableWhileTyping WRITE setDisableWhileTyping)
    Q_PROPERTY(bool disable_while_typing_support READ getDisableWhileTypingSupport WRITE setDisableWhileTypingSupport)
    Q_PROPERTY(bool has_touchpad READ getHasTouchpad WRITE setHasTouchpad)
    Q_PROPERTY(bool left_handed READ getLeftHanded WRITE setLeftHanded)
    Q_PROPERTY(double motion_acceleration READ getMotionAcceleration WRITE setMotionAcceleration)
    Q_PROPERTY(bool natural_scroll READ getNaturalScroll WRITE setNaturalScroll)
    Q_PROPERTY(int scroll_method READ getScrollMethod WRITE setScrollMethod)
    Q_PROPERTY(bool tap_to_click READ getTapToClick WRITE setTapToClick)
    Q_PROPERTY(bool tap_to_click_support READ getTapToClickSupport WRITE setTapToClickSupport)
    Q_PROPERTY(bool touchpad_enabled READ getTouchpadEnabled WRITE setTouchpadEnabled)

public:
    TouchPadManager();
    virtual ~TouchPadManager();

    static TouchPadManager *getInstance() { return m_instance; };

    static void globalInit();
    static void globalDeinit() { delete m_instance; };

public:
    // 设置点击触摸板的方式，当设置为BUTTON_AREAS(0)时，触摸板下方会划分左中右三块区域，
    // 这三块区域分别对应鼠标左/中/右键，当然你也可以通过1/2/3指轻击上方区域触发左/右/中键；
    // 当设置为CLICK_FINGER(1)时，只能通过1/2/3指轻击触摸板(不划分上下区域)来触发左键/右键/中键
    int getClickMethod() const { return m_clickMethod; };
    bool getClickMethodSupport() const { return m_clickMethodSupport; };
    // 在用键盘打字时触摸板无效
    bool getDisableWhileTyping() const { return m_disableWhileTyping; };
    bool getDisableWhileTypingSupport() const { return m_disableWhileTypingSupport; };
    // 是否存在触摸板设备
    bool getHasTouchpad() const { return m_hasTouchpad; };
    // 左手模式，如果为真，那么触摸板左键和右键效果切换，这里只会切换BUTTON_AREAS模式下点击
    // 触摸板下方左右区域触发的左键和右键效果，不会切换1/2指轻击触摸板触发的效果
    bool getLeftHanded() const { return m_leftHanded; };
    // 移动加速，范围为[-1,1]
    double getMotionAcceleration() const { return m_motionAcceleration; };
    // 自然滚动，如果设置为false，触摸板滑动方向与页面滚动方向相同，否则相反。具体的滚动方式可以通过设置scroll method来实现。
    bool getNaturalScroll() const { return m_naturalScroll; };
    // 滚动窗口的方式，分为twofinger, edge和button三种方式。
    // twofinger表示用两指滑动触摸板来达到滚动效果；edge表示滑动触摸板右边边缘来达到滚动效果；button表示操作键盘中间的红色按钮(部分机型存在)来达到滚动效果。
    int getScrollMethod() const { return m_scrollMethod; };
    // 敲击触摸板和鼠标按键之间的映射行为。默认情况下1/2/3指敲击分别对应鼠标左键/右键/中键
    bool getTapToClick() const { return m_tapToClick; };
    bool getTapToClickSupport() const { return m_tapToClickSupport; };
    // 开启或禁用所有触摸板
    bool getTouchpadEnabled() const { return m_touchpadEnabled; };

    void setClickMethod(int clickMethod);
    void setClickMethodSupport(bool clickMethodSupport);
    void setDisableWhileTyping(bool disableWhileTyping);
    void setDisableWhileTypingSupport(bool disableWhileTypingSupport);
    void setHasTouchpad(bool hasTouchpad);
    void setLeftHanded(bool leftHanded);
    void setMotionAcceleration(double motionAcceleration);
    void setNaturalScroll(bool naturalScroll);
    void setScrollMethod(int scrollMethod);
    void setTapToClick(bool tapToClick);
    void setTapToClickSupport(bool tapToClickSupport);
    void setTouchpadEnabled(bool touchpadEnabled);

public Q_SLOTS:  // METHODS
    void Reset();

private:
    void init();
    void initProperties();

    void processSettingsChanged(const QString &key);

    void setAllPropsToDevices();
    void setLeftHandedToDevices();
    void setDisableWhileTypingToDevices();
    void setTapToClickToDevices();
    void setClickMethodToDevices();
    void setScrollMethodToDevices();
    void setNaturalScrollToDevices();
    void setTouchpadEnabledToDevices();
    void setMotionAccelerationToDevices();
    void setPropToDevices(const QString &name, float value);
    void setPropToDevices(const QString &name, const QVector<bool> &values);

private:
    static TouchPadManager *m_instance;

    TouchPadAdaptor *m_adaptor;
    QGSettings *m_touchpadSettings;
    InputBackend *m_inputBackend;

    bool m_hasTouchpad;
    bool m_leftHanded;
    bool m_disableWhileTyping;
    bool m_tapToClick;
    int32_t m_clickMethod;
    int32_t m_scrollMethod;
    bool m_naturalScroll;
    bool m_touchpadEnabled;
    double m_motionAcceleration;

    bool m_disableWhileTypingSupport;
    bool m_tapToClickSupport;
    bool m_clickMethodSupport;
};
}  // namespace Kiran
