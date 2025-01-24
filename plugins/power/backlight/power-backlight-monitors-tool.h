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

#include "power-backlight-interface.h"

class QFileSystemWatcher;

namespace Kiran
{
class PowerBacklightMonitorsTool : public PowerBacklightMonitors
{
    Q_OBJECT

public:
    PowerBacklightMonitorsTool();
    virtual ~PowerBacklightMonitorsTool(){};

    static bool supportBacklight();

    virtual void init();
    // 获取所有显示器亮度设置对象
    virtual PowerBacklightAbsoluteList getMonitors() { return m_backlightMonitors; };

private:
    QString getBacklightDir();
    void processBrightnessChanged(const QString &path);

private:
    QFileSystemWatcher *m_brightnessWatcher;
    PowerBacklightAbsoluteList m_backlightMonitors;
};
}  // namespace Kiran