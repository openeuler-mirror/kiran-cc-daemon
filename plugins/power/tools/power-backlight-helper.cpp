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

#include "power-backlight-helper.h"
#include <unistd.h>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include "../wrapper/power-upower-device.h"
#include "../wrapper/power-upower.h"
#include "lib/base/base.h"

namespace Kiran
{
#define POWER_BACKLIGHT_SYS_PATH "/sys/class/backlight"

const QStringList PowerBacklightHelper::m_backlightSearchSubdirs = {
    "gmux_backlight",
    "nv_backlight",
    "nvidia_backlight",
    "intel_backlight",
    "dell_backlight",
    "asus_laptop",
    "toshiba",
    "eeepc",
    "eeepc-wmi",
    "thinkpad_screen",
    "acpi_video1",
    "mbp_backlight",
    "acpi_video0",
    "fujitsu-laptop",
    "sony",
    "samsung"};

PowerBacklightHelper::PowerBacklightHelper() : m_brightnessValue(-1)
{
    m_backlightDir = getBacklightFilepath();
    m_upowerClient = QSharedPointer<PowerUPower>::create();
}

PowerBacklightHelper::~PowerBacklightHelper()
{
}

void PowerBacklightHelper::init()
{
    if (m_backlightDir.length() > 0)
    {
        // auto fileName = QString("%1/brightness").arg(m_backlightDir);
        m_brightnessValue = getBrightnessValue();
    }

    m_upowerClient->init();
}
bool PowerBacklightHelper::supportBacklight()
{
    std::vector<uint32_t> deviceTypes = {UP_DEVICE_KIND_BATTERY, UP_DEVICE_KIND_UPS};

    for (auto deviceType : deviceTypes)
    {
        for (auto upowerDevice : m_upowerClient->getDevices())
        {
            auto& deviceProps = upowerDevice->getProps();
            if (deviceProps.type == deviceType &&
                deviceProps.isPresent)
            {
                return (m_brightnessValue >= 0);
            }
        }
    }

    return false;
}

int32_t PowerBacklightHelper::getBrightnessValue()
{
    RETURN_VAL_IF_FALSE(m_backlightDir.length() > 0, -1);

    auto fileName = QString("%1/brightness").arg(m_backlightDir);
    QFile file(fileName);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        KLOG_WARNING(power) << "Cannot access file" << fileName;
        return -1;
    }
    auto content = file.readAll();
    return content.toInt();
}

int32_t PowerBacklightHelper::getBrightnessMaxValue()
{
    RETURN_VAL_IF_FALSE(m_backlightDir.length() > 0, -1);

    auto fileName = QString("%1/max_brightness").arg(m_backlightDir);
    QFile file(fileName);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        KLOG_WARNING(power) << "Cannot access file" << fileName;
        return -1;
    }
    auto content = file.readAll();
    return content.toInt();
}

bool PowerBacklightHelper::setBrightnessValue(int32_t brightnessValue, QString& error)
{
    auto uid = getuid();
    auto euid = geteuid();
    if (uid != 0 || euid != 0)
    {
        error = QString(tr("This program can only be used by the root user"));
        return false;
    }

    auto pkexecUidStr = qgetenv("PKEXEC_UID");
    if (pkexecUidStr.isEmpty())
    {
        error = QString(tr("This program must only be run through pkexec"));
        return false;
    }

    auto fileName = QString("%1/brightness").arg(m_backlightDir);

    QFile file(fileName);
    if (file.open(QIODevice::ReadWrite | QIODevice::Text))
    {
        QTextStream out(&file);
        out << brightnessValue;
        out.flush();
    }
    else
    {
        error = QString(tr("Could not set the value of the backlight"));
        return false;
    }

    return true;
}

QString PowerBacklightHelper::getBacklightFilepath()
{
    // 先搜索指定的目录
    for (auto& subDir : m_backlightSearchSubdirs)
    {
        auto backlightDir = QString("%1/%2").arg(POWER_BACKLIGHT_SYS_PATH).arg(subDir);
        if (QFileInfo::exists(backlightDir))
        {
            return backlightDir;
        }
    }

    // 搜索不到的情况下选择第一个目录
    QDir dir(POWER_BACKLIGHT_SYS_PATH);
    auto fileInfoList = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
    if (fileInfoList.size() > 0)
    {
        return fileInfoList.first().absoluteFilePath();
    }
    return QString();
}

}  // namespace  Kiran
