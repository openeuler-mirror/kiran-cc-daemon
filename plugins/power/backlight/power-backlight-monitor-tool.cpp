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

#include "power-backlight-monitor-tool.h"
#include <QProcess>
#include "config.h"

namespace Kiran
{
#define POWER_BACKLIGHT_HELPER KCD_INSTALL_BINDIR "/kiran-power-backlight-helper"
PowerBacklightMonitorTool::PowerBacklightMonitorTool()
{
}

bool PowerBacklightMonitorTool::setBrightnessValue(int32_t brightness_value)
{
    QProcess process;
    QStringList arguments{POWER_BACKLIGHT_HELPER, "--set-brightness-value", QString::number(brightness_value)};
    process.start("pkexec", arguments);
    process.waitForFinished();

    auto command = QString("pkexec %1").arg(arguments.join(' '));
    if (process.exitCode() != 0)
    {
        KLOG_WARNING(power) << "Run command" << command << "failed, exit code is" << process.exitCode();
        return false;
    }
    else
    {
        KLOG_INFO(power) << "Run command" << command << "success";
        return true;
    }
}

int32_t PowerBacklightMonitorTool::getBrightnessValue()
{
    QProcess process;
    process.start(POWER_BACKLIGHT_HELPER, QStringList{"--get-brightness-value"});
    process.waitForFinished();

    if (process.exitCode() != 0)
    {
        auto command = QString("%1 --get-brightness-value").arg(POWER_BACKLIGHT_HELPER);
        KLOG_WARNING(power) << "Run command" << command << "failed, exit code is" << process.exitCode();
        return -1;
    }
    else
    {
        auto output = process.readAllStandardOutput();
        return output.toInt();
    }
}

bool PowerBacklightMonitorTool::getBrightnessRange(int32_t &min, int32_t &max)
{
    min = 0;
    max = 0;

    QProcess process;
    process.start(POWER_BACKLIGHT_HELPER, QStringList{"--get-max-brightness-value"});
    process.waitForFinished();

    if (process.exitCode() != 0)
    {
        auto command = QString("%1 --get-max-brightness-value").arg(POWER_BACKLIGHT_HELPER);
        KLOG_WARNING(power) << "Run command" << command << "failed, exit code is" << process.exitCode();
        return false;
    }
    else
    {
        auto output = process.readAllStandardOutput();
        max = output.toInt();
        KLOG_INFO(power) << "The birghtness range is" << min << "to" << max;
    }

    return true;
}

}  // namespace Kiran