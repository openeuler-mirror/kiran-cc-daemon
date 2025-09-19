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

#include "systeminfo-hardware.h"

#include <glibtop/mem.h>
#include <gudev/gudev.h>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegExp>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include "lib/base/base.h"

namespace Kiran
{
#define CPUINFO_CMD "/usr/bin/lscpu"
#define CPUINFO_FILE "/proc/cpuinfo"
#define CPUINFO_KEY_DELIMITER ':'
#define CPUINFO_KEY_MODEL "model name"
// 龙芯cpuinfo中为大写
#define CPUINFO_KEY_MODEL_LS "Model Name"
#define CPUINFO_KEY_PROCESSOR "processor"

#define MEMINFO_FILE "/proc/meminfo"
#define MEMINFO_KEY_DELIMITER ':'
#define MEMINFO_KEY_MEMTOTAL "MemTotal"

#define DISKINFO_CMD "/usr/bin/lsblk"

// 使用相对路径，避免使用绝对路径时因系统版本导致的错误
#define PCIINFO_CMD "lspci"
#define PCIINFO_KEY_DELIMITER ':'

SystemInfoHardware::SystemInfoHardware(QObject* parent) : QObject(parent),
                                                          m_memSizeLshw(0)
{
    m_lshwProcess = new QProcess(this);

    initMeminfoWithLshw();
}

void SystemInfoHardware::initMeminfoWithLshw()
{
    // 使用工具lshw获取硬件信息返回结果可能耗时1s左右，为避免用户读取数据延迟在初始化过程中调用一次即可
    connect(m_lshwProcess, SIGNAL(readyReadStandardOutput()), this, SLOT(processLshwStandardOutput()));
    connect(m_lshwProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(processLshwFinished(int, QProcess::ExitStatus)));
    m_lshwProcess->start("/usr/sbin/lshw", QStringList({"-json"}));
}

HardwareInfo SystemInfoHardware::getHardwareInfo()
{
    HardwareInfo hardwareInfo;
    hardwareInfo.cpuInfo = getCpuInfo();
    hardwareInfo.memInfo = getMemInfo();
    hardwareInfo.disksInfo = getDisksInfo();
    hardwareInfo.ethsInfo = getEthsInfo();
    hardwareInfo.graphicsInfo = getGraphicsInfo();
    return hardwareInfo;
}

CPUInfo SystemInfoHardware::mergeCpuInfos(const QVector<CPUInfo>& cpuInfos)
{
    CPUInfo cpuInfo;
    for (auto& iter : cpuInfos)
    {
        if (cpuInfo.model.isEmpty())
        {
            cpuInfo.model = iter.model;
        }
        if (cpuInfo.coresNumber == 0)
        {
            cpuInfo.coresNumber = iter.coresNumber;
        }
    }
    return cpuInfo;
}

CPUInfo SystemInfoHardware::getCpuInfo()
{
    QVector<CPUInfo> cpuInfos;
    cpuInfos.push_back(getCpuInfoByCmd());
    cpuInfos.push_back(readCpuInfoByConf());
    return mergeCpuInfos(cpuInfos);
}

CPUInfo SystemInfoHardware::getCpuInfoByCmd()
{
    // 低版本不支持-J选项

    QProcess process;
    process.setEnvironment(QStringList{"LANG=en_US.UTF-8"});
    process.start(CPUINFO_CMD, QStringList());
    process.waitForFinished();
    auto cmdOutput = process.readAllStandardOutput();

    auto kvList = formatToKVList(cmdOutput);
    RETURN_VAL_IF_TRUE(kvList.size() == 0, CPUInfo());

    CPUInfo cpuInfo;
    QString biosModelName;
    for (auto& key : kvList[0])
    {
        auto value = kvList[0][key];
        switch (shash(key.toLatin1().data()))
        {
        case "CPU(s)"_hash:
            cpuInfo.coresNumber = value.toInt();
            break;
        case "Model name"_hash:
            cpuInfo.model = value;
            break;
        case "BIOS Model name"_hash:
            biosModelName = value;
            break;
        default:
            break;
        }
    }

    if (cpuInfo.model.isEmpty() && !biosModelName.isEmpty())
    {
        cpuInfo.model = biosModelName;
    }

    return cpuInfo;
}

CPUInfo SystemInfoHardware::readCpuInfoByConf()
{
    CPUInfo cpuInfo;
    auto cpuMaps = parseInfoFile(CPUINFO_FILE, CPUINFO_KEY_DELIMITER);
    cpuInfo.model = cpuMaps[CPUINFO_KEY_MODEL];
    // 适配龙芯架构
    if (cpuInfo.model.isEmpty())
    {
        cpuInfo.model = cpuMaps[CPUINFO_KEY_MODEL_LS];
    }

    if (cpuMaps.find(CPUINFO_KEY_PROCESSOR) != cpuMaps.end())
    {
        cpuInfo.coresNumber = cpuMaps[CPUINFO_KEY_PROCESSOR].toInt() + 1;
    }
    return cpuInfo;
}

MemInfo SystemInfoHardware::getMemInfo()
{
    MemInfo memInfo;

    memInfo.totalSize = getMemorySizeWithDmi();
    memInfo.availableSize = getMemorySizeWithLibgtop();

    if (memInfo.totalSize == 0)
    {
        memInfo.totalSize = getMemorySizeWithLshw();
        KLOG_INFO(systeminfo) << "Get total size with lshw" << memInfo.totalSize;
    }

    if (memInfo.totalSize == 0)
    {
        memInfo.totalSize = memInfo.availableSize;
        KLOG_INFO(systeminfo) << "Get total size with libgtop" << memInfo.totalSize;
    }

    KLOG_INFO(systeminfo) << "Use total size is" << memInfo.totalSize << ", available size is" << memInfo.availableSize;
    return memInfo;
}

QMap<QString, QString> SystemInfoHardware::parseInfoFile(const QString& path, char delimiter)
{
    QMap<QString, QString> result;
    QFile file(path);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        KLOG_WARNING(systeminfo) << "Failed to open file" << path;
        return result;
    }

    auto contents = file.readAll();
    auto lines = contents.split('\n');
    for (auto& line : lines)
    {
        auto key = QString(line).section(delimiter, 0, 0).trimmed();
        auto value = QString(line).section(delimiter, 1).trimmed();
        CONTINUE_IF_TRUE(value.isEmpty());
        result[key] = value;
    }

    file.close();

    return result;
}

DiskInfoVec SystemInfoHardware::getDisksInfo()
{
    // 老版本lsblk不支持-J选项，所以这里不使用json格式
    DiskInfoVec disksInfo;
    QProcess process;
    process.setEnvironment(QStringList{"LANG=en_US.UTF-8"});
    process.setProgram(DISKINFO_CMD);
    process.setArguments(QStringList{"-d", "-b", "-P", "-o", "NAME,TYPE,SIZE,MODEL,VENDOR"});
    process.start();
    process.waitForFinished();
    auto cmdOutput = process.readAllStandardOutput();
    auto lines = cmdOutput.split('\n');
    QRegularExpression re(R"(NAME=\"([^\"]*)\" TYPE=\"([^\"]*)\" SIZE=\"(\d+)\" MODEL=\"([^\"]*)\" VENDOR=\"([^\"]*)\")");

    for (const auto& line : lines)
    {
        auto match = re.match(line);
        if (match.hasMatch())
        {
            auto name = match.captured(1);
            auto type = match.captured(2);
            auto size = match.captured(3);
            auto model = match.captured(4);
            auto vendor = match.captured(5);

            if (type == "disk" &&
                !name.isEmpty() &&
                !size.isEmpty() &&
                (!model.isEmpty() || !vendor.isEmpty()))
            {
                DiskInfo diskInfo;
                diskInfo.name = name;
                diskInfo.size = size.toLongLong();
                diskInfo.model = !model.isEmpty() ? model : name;
                diskInfo.vendor = !vendor.isEmpty() ? vendor : name;
                disksInfo.push_back(diskInfo);
            }
        }
    }

    return disksInfo;
}

EthInfoVec SystemInfoHardware::getEthsInfo()
{
    EthInfoVec ethsInfo;
    auto pcisInfo = getPcisByMajorClassID(PCIMajorClassID::PCI_MAJOR_CLASS_ID_NETWORK);

    for (auto& pciInfo : pcisInfo)
    {
        EthInfo ethInfo;
        ethInfo.model = pciInfo["Device"];
        ethInfo.vendor = pciInfo["Vendor"];
        ethsInfo.push_back(std::move(ethInfo));
    }
    return ethsInfo;
}

GraphicInfoVec SystemInfoHardware::getGraphicsInfo()
{
    GraphicInfoVec graphicsInfo;
    auto pcisInfo = getPcisByMajorClassID(PCIMajorClassID::PCI_MAJOR_CLASS_ID_DISPLAY);

    for (auto& pciInfo : pcisInfo)
    {
        GraphicInfo graphicInfo;
        graphicInfo.model = pciInfo["Device"];
        graphicInfo.vendor = pciInfo["Vendor"];
        graphicsInfo.push_back(std::move(graphicInfo));
    }
    return graphicsInfo;
}

KVList SystemInfoHardware::getPcisByMajorClassID(PCIMajorClassID majorClassID)
{
    QVector<uint32_t> fullClassIds;

    // 获取主类ID为majorClassID的fullClassIds列表，例如majorClassID为02，fullClassIds列表为[0201, 0202]
    {
        QProcess process;
        process.setEnvironment(QStringList{"LANG=en_US.UTF-8"});
        process.start(PCIINFO_CMD, QStringList{"-n"});
        process.waitForFinished();

        auto cmdOutput = process.readAllStandardOutput();
        auto lines = cmdOutput.split('\n');

        for (auto& line : lines)
        {
            auto lineStr = QString(line);
            char placehold1[50];
            uint32_t fullClassID;
            if (sscanf(lineStr.toLatin1().data(), "%49s %x:", placehold1, &fullClassID) == 2)
            {
                if ((fullClassID >> 8) == majorClassID)
                {
                    fullClassIds.push_back(fullClassID);
                }
            }
        }
    }

    // 如果为空则不执行下面的命令，否则会取到所有的PCI设备(没有了-d选项的限制)
    RETURN_VAL_IF_TRUE(fullClassIds.size() == 0, KVList());

    // 对fullClassIds去重
    std::sort(fullClassIds.begin(), fullClassIds.end());
    fullClassIds.erase(std::unique(fullClassIds.begin(), fullClassIds.end()), fullClassIds.end());

    // 根据fullClassIds列表获取设备相关信息
    QString fullOutputs;
    for (auto& fullClassID : fullClassIds)
    {
        auto fullClassIDHex = QString::number(fullClassID, 16);
        fullClassIDHex = fullClassIDHex.rightJustified(4, '0');

        KLOG_INFO(systeminfo) << "Execute command:" << QString("lspci -vmm -d ::%1").arg(fullClassIDHex);

        QProcess process;
        process.setEnvironment(QStringList{"LANG=en_US.UTF-8"});
        process.start(PCIINFO_CMD, QStringList{"-vmm", "-d", QString("::%1").arg(fullClassIDHex)});
        process.waitForFinished();
        auto cmdOutput = process.readAllStandardOutput();
        fullOutputs.append(cmdOutput);
    }

    if (fullOutputs.isEmpty())
    {
        KLOG_WARNING(systeminfo) << "Get empty pci info calss id" << majorClassID;
        return KVList();
    }

    return formatToKVList(fullOutputs);
}

KVList SystemInfoHardware::formatToKVList(const QString& contents)
{
    auto blocks = contents.split(QRegExp("\n\n"), Qt::SkipEmptyParts);
    KVList pcisInfo;
    for (auto& block : blocks)
    {
        QMap<QString, QString> pciInfo;
        auto lines = block.split('\n');
        for (auto& line : lines)
        {
            auto field1 = line.section(PCIINFO_KEY_DELIMITER, 0, 0).trimmed();
            auto field2 = line.section(PCIINFO_KEY_DELIMITER, 1).trimmed();
            CONTINUE_IF_TRUE(field2.isEmpty());
            pciInfo[field1] = field2;
        }
        if (pciInfo.size() > 0)
        {
            pcisInfo.push_back(std::move(pciInfo));
        }
    }
    return pcisInfo;
}

void SystemInfoHardware::parseLshwMemoryInfo()
{
    auto jsonRoot = QJsonDocument::fromJson(m_hardwareInfoLshw.toLatin1());
    auto jsonRootObject = jsonRoot.object();
    auto jsonChildren = jsonRootObject["children"].toArray();

    for (const auto& jsonChild : jsonChildren)
    {
        auto classVal = jsonChild.toObject()["class"].toString();
        auto descVal = jsonChild.toObject()["description"].toString();
        if (classVal == "memory" && (descVal == "System memory" || descVal == "System Memory"))
        {
            m_memSizeLshw = jsonChild.toObject()["size"].toString().toLongLong();
            KLOG_INFO(systeminfo) << "Find System memory size" << m_memSizeLshw;
            break;
        }
    }
}

int64_t SystemInfoHardware::getMemorySizeWithLshw()
{
    return m_memSizeLshw;
}

int64_t SystemInfoHardware::getMemorySizeWithLibgtop()
{
    glibtop_mem mem;
    glibtop_get_mem(&mem);

    return mem.total;
}

int64_t SystemInfoHardware::getMemorySizeWithDmi()
{
    g_autoptr(GUdevClient) client = NULL;
    g_autoptr(GUdevDevice) dmi = NULL;
    const gchar* const subsystems[] = {"dmi", NULL};
    uint64_t ramTotal = 0;
    uint64_t numRam = 0;

    client = g_udev_client_new(subsystems);
    dmi = g_udev_client_query_by_sysfs_path(client, "/sys/devices/virtual/dmi/id");
    if (!dmi)
    {
        KLOG_WARNING(systeminfo) << "Get dmi failed.";
        return 0;
    }

    numRam = g_udev_device_get_property_as_uint64(dmi, "MEMORY_ARRAY_NUM_DEVICES");
    for (uint64_t i = 0; i < numRam; i++)
    {
        QString prop = QString("MEMORY_DEVICE_%1_SIZE").arg(i);
        ramTotal += g_udev_device_get_property_as_uint64(dmi, prop.toLatin1().data());
    }

    return ramTotal;
}

void SystemInfoHardware::processLshwStandardOutput()
{
    m_hardwareInfoLshw.append(m_lshwProcess->readAllStandardOutput());
}

void SystemInfoHardware::processLshwFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitStatus != QProcess::NormalExit)
    {
        KLOG_WARNING(systeminfo) << "lshw process exit with abnormal status, exit code:" << exitCode;
        return;
    }

    parseLshwMemoryInfo();
}

}  // namespace Kiran