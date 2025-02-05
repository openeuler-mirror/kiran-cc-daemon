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

#include "systeminfo-manager.h"
#include <systeminfoadaptor.h>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include "lib/base/base.h"
#include "lib/base/polkit-proxy.h"
#include "systeminfo-hardware.h"
#include "systeminfo-i.h"
#include "systeminfo-software.h"

namespace Kiran
{
#define AUTH_SET_HOST_NAME "com.kylinsec.kiran.system-daemon.systeminfo.set-host-name"
SystemInfoManager::SystemInfoManager()
{
    m_adaptor = new SystemInfoAdaptor(this);
    m_software = new SystemInfoSoftware(this);
    m_hardware = new SystemInfoHardware(this);
}

SystemInfoManager* SystemInfoManager::m_instance = nullptr;
void SystemInfoManager::globalInit()
{
    m_instance = new SystemInfoManager();
    m_instance->init();
}

#define SEND_PROPERTY_NOTIFY(property, propertyHump)                          \
    QVariantMap changedProperties;                                            \
    changedProperties.insert(QStringLiteral(#property), get##propertyHump()); \
                                                                              \
    QDBusMessage signalMessage = QDBusMessage::createSignal(                  \
        SYSTEMINFO_OBJECT_PATH,                                               \
        QStringLiteral("org.freedesktop.DBus.Properties"),                    \
        QStringLiteral("PropertiesChanged"));                                 \
                                                                              \
    signalMessage.setArguments({                                              \
        SYSTEMINFO_DBUS_INTERFACE_NAME,                                       \
        changedProperties,                                                    \
        QStringList(),                                                        \
    });                                                                       \
    QDBusConnection::systemBus().send(signalMessage);

QString
SystemInfoManager::getHostName() const
{
    auto softwareInfo = m_software->getSoftwareInfo();
    return softwareInfo.hostName;
}

void SystemInfoManager::setHostName(const QString& hostName)
{
    m_software->setHostName(hostName);
    SEND_PROPERTY_NOTIFY(host_name, HostName);
}
QString SystemInfoManager::GetSystemInfo(int type)
{
    QJsonObject values;

    switch (type)
    {
    case SystemInfoType::SYSTEMINFO_TYPE_SOFTWARE:
    {
        auto softwareInfo = m_software->getSoftwareInfo();
        values["kernal_name"] = softwareInfo.kernelName;
        values["host_name"] = softwareInfo.hostName;
        values["kernel_release"] = softwareInfo.kernelRelease;
        values["kernel_version"] = softwareInfo.kernelVersion;
        values["arch"] = softwareInfo.arch;
        values["product_name"] = softwareInfo.productName;
        values["product_release"] = softwareInfo.productRelease;
        break;
    }
    case SystemInfoType::SYSTEMINFO_TYPE_HARDWARE:
    {
        auto hardwareInfo = m_hardware->getHardwareInfo();
        QJsonObject jsonCpu;
        jsonCpu["model"] = hardwareInfo.cpuInfo.model;
        jsonCpu["cores_number"] = hardwareInfo.cpuInfo.coresNumber;
        values.insert("cpu", jsonCpu);

        QJsonObject jsonMem;
        jsonMem["total_size"] = qlonglong(hardwareInfo.memInfo.totalSize);
        jsonMem["available_size"] = qlonglong(hardwareInfo.memInfo.availableSize);
        values.insert("mem", jsonMem);

        QJsonArray disksValue;
        for (uint32_t i = 0; i < hardwareInfo.disksInfo.size(); ++i)
        {
            QJsonObject diskValue;
            diskValue["name"] = hardwareInfo.disksInfo[i].name;
            diskValue["model"] = hardwareInfo.disksInfo[i].model;
            diskValue["vendor"] = hardwareInfo.disksInfo[i].vendor;
            diskValue["size"] = qlonglong(hardwareInfo.disksInfo[i].size);
            disksValue.append(diskValue);
        }
        values.insert("disks", disksValue);

        QJsonArray ethsValue;
        for (uint32_t i = 0; i < hardwareInfo.ethsInfo.size(); ++i)
        {
            QJsonObject ethValue;
            ethValue["model"] = hardwareInfo.ethsInfo[i].model;
            ethValue["vendor"] = hardwareInfo.ethsInfo[i].vendor;
            ethsValue.append(ethValue);
        }
        values.insert("eths", ethsValue);

        QJsonArray graphicsValue;
        for (uint32_t i = 0; i < hardwareInfo.graphicsInfo.size(); ++i)
        {
            QJsonObject graphicValue;
            graphicValue["model"] = hardwareInfo.graphicsInfo[i].model;
            graphicValue["vendor"] = hardwareInfo.graphicsInfo[i].vendor;
            graphicsValue.append(graphicValue);
        }
        values.insert("graphics", graphicsValue);
        break;
    }
    default:
        DBUS_ERROR_REPLY_AND_RETVAL(QString(), CCErrorCode::ERROR_SYSTEMINFO_TYPE_INVALID);
    }

    return QJsonDocument(values).toJson(QJsonDocument::Compact);
}

void SystemInfoManager::SetHostName(const QString& hostName)
{
    PolkitProxy::getDefault()->checkAuthorization(AUTH_SET_HOST_NAME,
                                                  true,
                                                  this->message(),
                                                  std::bind(&SystemInfoManager::processHostNameChanged, this, std::placeholders::_1, hostName));
}

void SystemInfoManager::init()
{
    auto systemConnection = QDBusConnection::systemBus();
    if (!systemConnection.registerService(SYSTEMINFO_DBUS_NAME))
    {
        KLOG_WARNING() << "Failed to register dbus name: " << SYSTEMINFO_DBUS_NAME;
        return;
    }

    if (!systemConnection.registerObject(SYSTEMINFO_OBJECT_PATH, SYSTEMINFO_DBUS_INTERFACE_NAME, this))
    {
        KLOG_ERROR() << "Can't register object:" << systemConnection.lastError();
        return;
    }
}

void SystemInfoManager::processHostNameChanged(const QDBusMessage& message, const QString& hostName)
{
    setHostName(hostName);
    QDBusConnection::systemBus().send(message.createReply());
}

}  // namespace Kiran