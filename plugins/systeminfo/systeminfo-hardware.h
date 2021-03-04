/**
 * @file          /kiran-cc-daemon/plugins/systeminfo/systeminfo-hardware.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#include <json/json.h>

#include "lib/base/base.h"

namespace Kiran
{
// CPU信息
struct CPUInfo
{
    CPUInfo() : cores_number(0){};
    // CPU型号
    std::string model;
    // CPU核心数
    int32_t cores_number;
};

// 内存信息
struct MemInfo
{
    MemInfo() : total_size(0){};
    // 内存总大小
    int64_t total_size;
};

// 硬盘信息
struct DiskInfo
{
    DiskInfo() : size(0){};
    // 名称
    std::string name;
    // 型号
    std::string model;
    // 设备厂商
    std::string vendor;
    // 大小
    int64_t size;
};
using DiskInfoVec = std::vector<DiskInfo>;

// 网卡信息
struct EthInfo
{
    EthInfo() = default;
    // 型号
    std::string model;
    // 设备厂商
    std::string vendor;
};
using EthInfoVec = std::vector<EthInfo>;

// 显卡信息
struct GraphicInfo
{
    GraphicInfo() = default;
    // 型号
    std::string model;
    // 设备厂商
    std::string vendor;
};
using GraphicInfoVec = std::vector<GraphicInfo>;

struct HardwareInfo
{
    HardwareInfo() = default;

    CPUInfo cpu_info;
    MemInfo mem_info;
    DiskInfoVec disks_info;
    EthInfoVec eths_info;
    GraphicInfoVec graphics_info;
};

using PCIsInfo = std::vector<std::map<std::string, std::string>>;

class SystemInfoHardware
{
public:
    SystemInfoHardware();
    virtual ~SystemInfoHardware(){};

    // 获取硬件信息，包括cpu、内存、硬盘、网卡和显卡等信息
    HardwareInfo get_hardware_info();

private:
    CPUInfo get_cpu_info();
    // 通过lscpu命令获取
    CPUInfo get_cpu_info_by_cmd();
    // 如果命令获取失败，则直接读取配置文件
    CPUInfo read_cpu_info_by_conf();

    MemInfo get_mem_info();
    void set_env();
    std::map<std::string, std::string> parse_info_file(const std::string &path, char delimiter);

    DiskInfoVec get_disks_info();

    EthInfoVec get_eths_info();
    GraphicInfoVec get_graphics_info();

    Json::Value run_command(std::vector<std::string> &argv, bool add_bracket = false);
};
}  // namespace Kiran