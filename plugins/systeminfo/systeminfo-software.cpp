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

#include "systeminfo-software.h"
#include <sys/utsname.h>
#include <QProcess>
#include "lib/base/base.h"

namespace Kiran
{
#define SET_HOSTNAME_CMD "/usr/bin/hostnamectl"

SystemInfoSoftware::SystemInfoSoftware(QObject *parent) : QObject(parent)
{
}

SoftwareInfo SystemInfoSoftware::getSoftwareInfo()
{
    SoftwareInfo softwareInfo;
    readKernelInfo(softwareInfo);
    readProductInfo(softwareInfo);
    return softwareInfo;
}

bool SystemInfoSoftware::setHostName(const QString &hostName)
{
    QProcess process;
    process.setProgram(SET_HOSTNAME_CMD);
    process.setArguments(QStringList({"set-hostname", hostName}));
    process.start();
    process.waitForFinished();

    if (process.exitStatus() != QProcess::ExitStatus::NormalExit)
    {
        KLOG_WARNING(systeminfo) << "Call hostnamectl failed, exit code:" << process.exitCode();
        return false;
    }
    else
    {
        QString command = QString("%1 set-hostname %2").arg(SET_HOSTNAME_CMD).arg(hostName);
        KLOG_INFO(systeminfo) << "Run command" << command << "success";
    }
    return true;
}

bool SystemInfoSoftware::readKernelInfo(SoftwareInfo &softwareInfo)
{
    struct utsname utsName;

    auto retval = uname(&utsName);
    if (retval < 0)
    {
        KLOG_WARNING(systeminfo) << "Call uname() failed:" << strerror(errno);
        return false;
    }

    softwareInfo.kernelName = utsName.sysname;
    softwareInfo.hostName = utsName.nodename;
    softwareInfo.kernelRelease = utsName.release;
    softwareInfo.kernelVersion = utsName.version;
    softwareInfo.arch = utsName.machine;

    return true;
}

void SystemInfoSoftware::readProductInfo(SoftwareInfo &softwareInfo)
{
    softwareInfo.productName = getReleaseInfo("lsb_release", QStringList({"-i", "-s"}));
    softwareInfo.productRelease = getReleaseInfo("lsb_release", QStringList({"-d", "-s"}));
}

QString SystemInfoSoftware::getReleaseInfo(const QString &program, const QStringList &arguments)
{
    QString output;
    QProcess process;
    process.start(program, arguments);
    process.waitForFinished();
    if (process.exitStatus() != QProcess::ExitStatus::NormalExit)
    {
        KLOG_WARNING(systeminfo) << "Call lsb_release failed, exit code:" << process.exitCode();
    }
    else
    {
        output = process.readAllStandardOutput().trimmed();
        if (output.size() > 1 && output.front() == '"' && output.back() == '"')
        {
            output.remove(0, 1);
            output.chop(1);
        }
    }
    return output;
}
}  // namespace Kiran