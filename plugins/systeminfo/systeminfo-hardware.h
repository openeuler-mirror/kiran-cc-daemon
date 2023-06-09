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
    MemInfo() : total_size(0), available_size(0){};
    // 内存总大小
    int64_t total_size;
    // 内存可用大小
    int64_t available_size;
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

// 具体值参考/usr/share/hwdata/pci.ids文件
enum PCIMajorClassID
{
    PCI_MAJOR_CLASS_ID_UNCLASSIFIED = 0,
    PCI_MAJOR_CLASS_ID_MASS_STORAGE = 1,
    PCI_MAJOR_CLASS_ID_NETWORK = 2,
    PCI_MAJOR_CLASS_ID_DISPLAY = 3,
};

using KVList = std::vector<std::map<std::string, std::string>>;

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
    //合并读取信息
    CPUInfo merge_cpu_infos(const std::vector<CPUInfo> &cpu_infos);

    MemInfo get_mem_info();
    void set_env();
    std::map<std::string, std::string> parse_info_file(const std::string &path, char delimiter);

    DiskInfoVec get_disks_info();

    EthInfoVec get_eths_info();
    GraphicInfoVec get_graphics_info();

    KVList get_pcis_by_major_class_id(PCIMajorClassID major_class_id);

    KVList format_to_kv_list(const std::string &contents);

    void init_meminfo_with_lshw();

    void parse_lshw_memory_info();

    bool on_lshw_output(Glib::IOCondition io_condition, Glib::RefPtr<Glib::IOChannel> io_channel);

    void on_child_watch(GPid pid, int child_status);

    int64_t get_memory_size_with_lshw();

    int64_t get_memory_size_with_libgtop();

    int64_t get_memory_size_with_dmi();

private:
    Glib::ustring hardware_info_lshw;
    int64_t mem_size_lshw;

    Glib::RefPtr<Glib::IOChannel> out_io_channel_;
    Glib::RefPtr<Glib::IOSource> out_io_source_;
    sigc::connection out_io_connection_;
    sigc::connection watch_child_connection_;
    Glib::Pid child_pid_;
};
}  // namespace Kiran