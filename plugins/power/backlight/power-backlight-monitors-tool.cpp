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

#include "power-backlight-monitors-tool.h"
#include <QFileSystemWatcher>
#include <QProcess>
#include "config.h"
#include "power-backlight-monitor-tool.h"

namespace Kiran
{
#define POWER_BACKLIGHT_HELPER KCD_INSTALL_BINDIR "/kiran-power-backlight-helper"

PowerBacklightMonitorsTool::PowerBacklightMonitorsTool()
{
    m_brightnessWatcher = new QFileSystemWatcher(this);

    auto backlightDir = getBacklightDir();
    if (!backlightDir.isEmpty())
    {
        auto fileName = QString("%1/%2").arg(backlightDir, "brightness");
        m_brightnessWatcher->addPath(fileName);
    }

    connect(m_brightnessWatcher, &QFileSystemWatcher::fileChanged, this, &PowerBacklightMonitorsTool::processBrightnessChanged);
}

bool PowerBacklightMonitorsTool::supportBacklight()
{
    QProcess process;
    process.start(POWER_BACKLIGHT_HELPER, QStringList{"--support-backlight"});
    process.waitForFinished();

    if (process.exitCode() != 0)
    {
        auto command = QString("%1 --support-backlight").arg(POWER_BACKLIGHT_HELPER);
        KLOG_WARNING(power) << "Run command" << command << "failed, exit code is" << process.exitCode();
        return false;
    }
    else
    {
        auto output = process.readAllStandardOutput();
        return (output.toInt() == 1);
    }
}

void PowerBacklightMonitorsTool::init()
{
    m_backlightMonitors.clear();
    m_backlightMonitors.push_back(QSharedPointer<PowerBacklightMonitorTool>::create());
}

QString PowerBacklightMonitorsTool::getBacklightDir()
{
    QProcess process;
    process.start(POWER_BACKLIGHT_HELPER, QStringList{"--get-backlight-directory"});
    process.waitForFinished();

    if (process.exitCode() != 0)
    {
        auto command = QString("%1 --get-backlight-directory").arg(POWER_BACKLIGHT_HELPER);
        KLOG_WARNING(power) << "Run command" << command << "failed, exit code is" << process.exitCode();
        return QString();
    }
    else
    {
        return process.readAllStandardOutput();
    }
}

void PowerBacklightMonitorsTool::processBrightnessChanged(const QString &path)
{
    Q_EMIT brightnessChanged();
}

}  // namespace Kiran