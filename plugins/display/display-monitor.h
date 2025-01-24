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

#include <kscreen/types.h>
#include <QDBusContext>
#include "dbus-types.h"

class MonitorAdaptor;

namespace KScreen
{
class Output;
}

namespace Kiran
{

class DisplayMonitor : public QObject,
                       protected QDBusContext
{
    Q_OBJECT

    Q_PROPERTY(bool connected READ getConnected WRITE setConnected)
    Q_PROPERTY(uint current_mode READ getCurrentMode WRITE setCurrentMode)
    Q_PROPERTY(bool enabled READ getEnabled WRITE setEnabled)
    Q_PROPERTY(uint id READ getID WRITE setID)
    Q_PROPERTY(quint32List modes READ getModes)
    Q_PROPERTY(QString name READ getName WRITE setName)
    Q_PROPERTY(int npreferred READ getNPreferred)
    Q_PROPERTY(ushort reflect READ getReflect WRITE setReflect)
    Q_PROPERTY(quint16List reflects READ getReflects WRITE setReflects)
    Q_PROPERTY(ushort rotation READ getRotation WRITE setRotation)
    Q_PROPERTY(quint16List rotations READ getRotations WRITE setRotations)
    Q_PROPERTY(int x READ getX WRITE setX)
    Q_PROPERTY(int y READ getY WRITE setY)

public:
    DisplayMonitor() = delete;
    DisplayMonitor(const KScreen::OutputPtr monitorInfo);
    virtual ~DisplayMonitor();

public:
    bool getConnected() const;
    uint getCurrentMode() const;
    bool getEnabled() const;
    uint getID() const;
    quint32List getModes() const;
    QString getName() const;
    int getNPreferred() const;
    ushort getReflect() const;
    quint16List getReflects() const;
    ushort getRotation() const;
    quint16List getRotations() const;
    int getX() const;
    int getY() const;

    void setConnected(bool connected);
    void setCurrentMode(uint currentMode);
    void setEnabled(bool enabled);
    void setID(uint id);
    void setName(const QString &name);
    void setReflect(ushort reflect);
    void setReflects(quint16List reflects);
    void setRotation(ushort rotation);
    void setRotations(quint16List rotations);
    void setX(int x);
    void setY(int y);

public Q_SLOTS:
    void Enable(bool enabled);
    DisplayModesStu GetCurrentMode();
    // 获取当前monitor可用的mode列表，列表元素包括ID，分辨率width，分辨率height和刷新率。
    ListDisplayModesStu ListModes();
    // 获取当前monitor最佳的mode列表，一般情况下只有一个元素，列表元素包括ID，分辨率width，分辨率height和刷新率。
    ListDisplayModesStu ListPreferredModes();
    // 设置mode，最终设置的刷新率可能和refresh_rate存在差异，会遍历一个最接近的值进行设置
    void SetMode(uint width, uint height, double refresh_rate);
    // 设置当前使用的mode，参数为可用的mode列表的下标
    void SetModeById(uint id);
    // 通过分辨率设置当前使用的mode
    void SetModeBySize(uint width, uint height);
    // 设置monitor在屏幕中显示的位置
    void SetPosition(int x, int y);
    // 设置翻转，参数为reflects列表的下标
    void SetReflect(ushort reflect);
    // 设置旋转，参数为rotations列表的下标
    void SetRotation(ushort rotation);

public:
    KScreen::OutputPtr getOutput() const { return m_output; }
    void update(const KScreen::OutputPtr output);

    void dbusRegister();
    void dbusUnregister();

    // 获取uid
    QString getUID();
    // 获取object_path
    QString getObjectPath() { return m_objectPath; };
    // 获取最佳的mode,一般时可用mode列表的第一个
    KScreen::ModePtr getBestMode();

    // 通过大小获取可用的mode列表
    QVector<KScreen::ModePtr> getModesBySize(uint32_t width, uint32_t height);
    // 找到分辨率相同的mode列表，并匹配刷新率最接近的mode
    KScreen::ModePtr matchBestMode(uint32_t width, uint32_t height, double refresh_rate);

private:
    MonitorAdaptor *m_monitorAdaptor;
    QString m_objectPath;
    KScreen::OutputPtr m_output;
};

}  // namespace Kiran
