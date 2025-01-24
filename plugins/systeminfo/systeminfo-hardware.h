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

#include <QMap>
#include <QObject>
#include <QProcess>
#include <QVector>

namespace Kiran
{
// CPU信息
struct CPUInfo
{
    CPUInfo() : coresNumber(0){};
    // CPU型号
    QString model;
    // CPU核心数
    int32_t coresNumber;
};

// 内存信息
struct MemInfo
{
    MemInfo() : totalSize(0), availableSize(0){};
    // 内存总大小
    int64_t totalSize;
    // 内存可用大小
    int64_t availableSize;
};

// 硬盘信息
struct DiskInfo
{
    DiskInfo() : size(0){};
    // 名称
    QString name;
    // 型号
    QString model;
    // 设备厂商
    QString vendor;
    // 大小
    int64_t size;
};
using DiskInfoVec = QVector<DiskInfo>;

// 网卡信息
struct EthInfo
{
    EthInfo() = default;
    // 型号
    QString model;
    // 设备厂商
    QString vendor;
};
using EthInfoVec = QVector<EthInfo>;

// 显卡信息
struct GraphicInfo
{
    GraphicInfo() = default;
    // 型号
    QString model;
    // 设备厂商
    QString vendor;
};
using GraphicInfoVec = QVector<GraphicInfo>;

struct HardwareInfo
{
    HardwareInfo() = default;

    CPUInfo cpuInfo;
    MemInfo memInfo;
    DiskInfoVec disksInfo;
    EthInfoVec ethsInfo;
    GraphicInfoVec graphicsInfo;
};

// 具体值参考/usr/share/hwdata/pci.ids文件
enum PCIMajorClassID
{
    PCI_MAJOR_CLASS_ID_UNCLASSIFIED = 0,
    PCI_MAJOR_CLASS_ID_MASS_STORAGE = 1,
    PCI_MAJOR_CLASS_ID_NETWORK = 2,
    PCI_MAJOR_CLASS_ID_DISPLAY = 3,
};

using KVList = QVector<QMap<QString, QString>>;

class SystemInfoHardware : public QObject
{
    Q_OBJECT

public:
    SystemInfoHardware(QObject *parent = nullptr);
    virtual ~SystemInfoHardware(){};

    // 获取硬件信息，包括cpu、内存、硬盘、网卡和显卡等信息
    HardwareInfo getHardwareInfo();

private:
    CPUInfo getCpuInfo();
    // 通过lscpu命令获取
    CPUInfo getCpuInfoByCmd();
    // 如果命令获取失败，则直接读取配置文件
    CPUInfo readCpuInfoByConf();
    // 合并读取信息
    CPUInfo mergeCpuInfos(const QVector<CPUInfo> &cpu_infos);

    MemInfo getMemInfo();
    QMap<QString, QString> parseInfoFile(const QString &path, char delimiter);

    DiskInfoVec getDisksInfo();
    EthInfoVec getEthsInfo();
    GraphicInfoVec getGraphicsInfo();
    KVList getPcisByMajorClassID(PCIMajorClassID majorClassID);
    KVList formatToKVList(const QString &contents);
    void initMeminfoWithLshw();
    void parseLshwMemoryInfo();

    int64_t getMemorySizeWithLshw();
    int64_t getMemorySizeWithLibgtop();
    int64_t getMemorySizeWithDmi();

private Q_SLOTS:
    void processLshwStandardOutput();
    void processLshwFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    int64_t m_memSizeLshw;
    QProcess *m_lshwProcess;
    QString m_hardwareInfoLshw;
};
}  // namespace Kiran