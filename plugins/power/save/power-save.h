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

#include <power-i.h>
#include <QObject>
#include <QSharedPointer>

class QGSettings;

namespace KScreen
{
class Dpms;
}

namespace Kiran
{
class PowerWrapperManager;
class PowerBacklight;
class PowerSaveComputer;
class PowerProfiles;

class PowerSave : public QObject
{
    Q_OBJECT

public:
    PowerSave(PowerWrapperManager* wrapperManager, PowerBacklight* backlight);
    virtual ~PowerSave();

    static PowerSave* getInstance() { return m_instance; };

    static void globalInit(PowerWrapperManager* wrapperManager, PowerBacklight* backlight);

    static void globalDeinit() { delete m_instance; };

    // 执行节能动作
    bool doSave(PowerAction action, QString& error);

    // 显示器是否处于变暗状态
    bool isDisplayDimmed();
    // 执行显示器变暗，如果已经处于变暗状态，则返回失败
    bool doDisplayDimmed();
    // 执行显示器恢复变暗操作
    void doDisplayRestoreDimmed();

    // 执行CPU节能操作
    void doCpuSaver();
    // 执行恢复CPU节能操作
    void doCpuRestoreSaver();

private:
    void init();

    void processKbdBrightnessChanged(int32_t brightnessPercentage);
    void processMonitorBrightnessChanged(int32_t brightnessPercentage);
    void processActiveProfileChanged(int32_t profileMode);

private:
    static PowerSave* m_instance;
    PowerWrapperManager* m_wrapperManager;
    PowerBacklight* m_backlight;

    KScreen::Dpms* m_dpms;
    QGSettings* m_powerSettings;
    QSharedPointer<PowerProfiles> m_profiles;

    // 记录亮度变暗前的亮度值
    int32_t m_kbdRestoreBrightness;
    int32_t m_monitorRestoreBrightness;
    // 记录变暗的时间
    time_t m_displayDimmedTimestamp;

    // 记录节能之前的状态
    uint32_t m_cpuSaverCookie;
    time_t m_cpuSaverTimestamp;

    PowerSaveComputer* m_saveComputer;
};
}  // namespace Kiran