/**
 * @file          /kiran-cc-daemon/plugins/systeminfo/systeminfo-hardware.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#include "plugins/systeminfo/systeminfo-hardware.h"

#include <fstream>

namespace Kiran
{
#define CPUINFO_CMD "/usr/bin/lscpu"
#define CPUINFO_FILE "/proc/cpuinfo"
#define CPUINFO_KEY_DELIMITER ':'
#define CPUINFO_KEY_MODEL "model name"
#define CPUINFO_KEY_PROCESSOR "processor"

#define MEMINFO_FILE "/proc/meminfo"
#define MEMINFO_KEY_DELIMITER ':'
#define MEMINFO_KEY_MEMTOTAL "MemTotal"

#define DISKINFO_CMD "/usr/bin/lsblk"

#define HWINFO_CMD "/usr/sbin/lshw"

SystemInfoHardware::SystemInfoHardware()
{
}

HardwareInfo SystemInfoHardware::get_hardware_info()
{
    SETTINGS_PROFILE("");

    HardwareInfo hardware_info;
    hardware_info.cpu_info = this->get_cpu_info();
    hardware_info.mem_info = this->get_mem_info();
    hardware_info.disks_info = this->get_disks_info();
    hardware_info.eths_info = this->get_eths_info();
    hardware_info.graphics_info = this->get_graphics_info();
    return hardware_info;
}

CPUInfo SystemInfoHardware::get_cpu_info()
{
    auto cpu_info = this->get_cpu_info_by_cmd();

    if (cpu_info.cores_number == 0)
    {
        cpu_info = this->read_cpu_info_by_conf();
    }

    return cpu_info;
}

CPUInfo SystemInfoHardware::get_cpu_info_by_cmd()
{
    CPUInfo cpu_info;
    std::vector<std::string> argv{CPUINFO_CMD, "-J"};

    try
    {
        auto values = this->run_command(argv);
        RETURN_VAL_IF_TRUE(values.isNull(), CPUInfo());
        auto infos = values["lscpu"];

        for (auto& fields : infos)
        {
            switch (shash(fields["field"].asString().c_str()))
            {
            case "CPU(s):"_hash:
                cpu_info.cores_number = strtol(fields["data"].asString().c_str(), NULL, 0);
                break;
            case "Model name:"_hash:
                cpu_info.model = fields["data"].asString();
                break;
            default:
                break;
            }
        }
    }
    catch (const std::exception& e)
    {
        LOG_WARNING("%s.", e.what());
        return CPUInfo();
    }
    return cpu_info;
}

CPUInfo SystemInfoHardware::read_cpu_info_by_conf()
{
    CPUInfo cpu_info;
    auto cpu_maps = this->parse_info_file(CPUINFO_FILE, CPUINFO_KEY_DELIMITER);

    cpu_info.model = cpu_maps[CPUINFO_KEY_MODEL];
    if (cpu_maps.find(CPUINFO_KEY_PROCESSOR) != cpu_maps.end())
    {
        cpu_info.cores_number = strtol(cpu_maps[CPUINFO_KEY_PROCESSOR].c_str(), NULL, 0) + 1;
    }
    return cpu_info;
}

MemInfo SystemInfoHardware::get_mem_info()
{
    MemInfo mem_info;
    auto mem_maps = this->parse_info_file(MEMINFO_FILE, MEMINFO_KEY_DELIMITER);
    auto total_fields = StrUtils::split_with_char(mem_maps[MEMINFO_KEY_MEMTOTAL], ' ', true);
    if (total_fields.size() == 2)
    {
        mem_info.total_size = strtoll(total_fields[0].c_str(), NULL, 0) * 1024;
    }
    else
    {
        LOG_WARNING("Not found valid record: %s.", mem_maps[MEMINFO_KEY_MEMTOTAL].c_str());
    }
    return mem_info;
}

void SystemInfoHardware::set_env()
{
    Glib::setenv("LANG", "en_US.UTF-8", true);
}

std::map<std::string, std::string> SystemInfoHardware::parse_info_file(const std::string& path, char delimiter)
{
    std::map<std::string, std::string> result;
    std::fstream fs(path.c_str(), std::fstream::in);
    // meminfo和cpuinfo文件中的内容是动态生成的，因此不能通过获取文件大小的方式来分配缓存
    char buffer[BUFSIZ];

    SCOPE_EXIT({ fs.close(); });

    if (fs.fail())
    {
        LOG_WARNING("Failed to open file %s.", path.c_str());
        return std::map<std::string, std::string>();
    }

    while (!fs.eof())
    {
        fs.getline(buffer, BUFSIZ);

        // LOG_DEBUG("buffer: %s.", buffer);

        RETURN_VAL_IF_TRUE(fs.fail(), result);

        auto fields = StrUtils::split_with_char(std::string(buffer), delimiter);

        if (fields.size() != 2)
        {
            continue;
        }

        result[StrUtils::trim(fields[0])] = StrUtils::trim(fields[1]);
    }

    return result;
}

DiskInfoVec SystemInfoHardware::get_disks_info()
{
    DiskInfoVec disks_info;
    std::vector<std::string> argv{DISKINFO_CMD, "-d", "-b", "-o", "NAME,TYPE,SIZE,MODEL,VENDOR"};

    std::string cmd_output;
    try
    {
        Glib::spawn_sync("",
                         argv,
                         Glib::SPAWN_DEFAULT,
                         sigc::mem_fun(this, &SystemInfoHardware::set_env),
                         &cmd_output);
    }
    catch (const Glib::Error& e)
    {
        LOG_WARNING("%s", e.what().c_str());
        return disks_info;
    }

    auto lines = StrUtils::split_lines(cmd_output);

    // 排除第一行表头
    for (size_t i = 1; i < lines.size(); ++i)
    {
        auto fields = StrUtils::split_with_char(lines[i], ' ', true);
        if (fields.size() >= 5 && fields[1] == "disk")
        {
            DiskInfo disk_info;
            disk_info.name = fields[0];
            disk_info.size = strtoll(fields[2].c_str(), NULL, 0);
            disk_info.model = fields[3];
            disk_info.vendor = fields[4];
            disks_info.push_back(disk_info);
        }
    }

    return disks_info;

    /*DiskInfoVec disks_info;
    std::vector<std::string> argv{DISKINFO_CMD, "-d", "-b", "-J", "-o", "NAME,TYPE,SIZE,MODEL,VENDOR"};

    try
    {
        auto values = this->run_command(argv);
        RETURN_VAL_IF_TRUE(values.isNull(), DiskInfoVec());
        auto block_devices = values["blockdevices"];

        for (auto& block_device : block_devices)
        {
            if (block_device["type"] != "disk")
            {
                continue;
            }

            DiskInfo disk_info;
            disk_info.name = block_device["name"].asString();
            disk_info.size = strtoll(block_device["size"].asString().c_str(), NULL, 0);
            disk_info.model = block_device["model"].asString();
            disk_info.vendor = StrUtils::trim(block_device["vendor"].asString());
            disks_info.push_back(disk_info);
        }
    }
    catch (const std::exception& e)
    {
        LOG_WARNING("%s.", e.what());
        return DiskInfoVec();
    }
    return disks_info;*/
}

EthInfoVec SystemInfoHardware::get_eths_info()
{
    // 部分机型的网卡可能是USB接口，但lsusb无法通过类型来区分那些是网卡，因此这里先不做处理。

    EthInfoVec eths_info;
    std::vector<std::string> argv{HWINFO_CMD, "-json", "-C", "network"};

    try
    {
        auto eths_json = this->run_command(argv, true);
        RETURN_VAL_IF_TRUE(eths_json.isNull(), EthInfoVec());

        for (auto& eth_json : eths_json)
        {
            EthInfo eth_info;
            eth_info.model = eth_json["product"].asString();
            eth_info.vendor = eth_json["vendor"].asString();
            eths_info.push_back(eth_info);
        }
    }
    catch (const std::exception& e)
    {
        LOG_WARNING("%s.", e.what());
        return EthInfoVec();
    }

    return eths_info;
}

GraphicInfoVec SystemInfoHardware::get_graphics_info()
{
    GraphicInfoVec graphics_info;
    std::vector<std::string> argv{HWINFO_CMD, "-json", "-C", "display"};

    try
    {
        auto graphics_json = this->run_command(argv, true);
        RETURN_VAL_IF_TRUE(graphics_json.isNull(), GraphicInfoVec());

        for (auto& graphic_json : graphics_json)
        {
            GraphicInfo graphic_info;
            graphic_info.model = graphic_json["product"].asString();
            graphic_info.vendor = graphic_json["vendor"].asString();
            graphics_info.push_back(graphic_info);
        }
    }
    catch (const std::exception& e)
    {
        LOG_WARNING("%s.", e.what());
        return GraphicInfoVec();
    }

    return graphics_info;
}

Json::Value SystemInfoHardware::run_command(std::vector<std::string>& argv, bool add_bracket)
{
    LOG_DEBUG("cmdline: %s, add_bracket: %d.", StrUtils::join(argv, " ").c_str(), add_bracket);

    std::string cmd_output;
    try
    {
        Glib::spawn_sync("",
                         argv,
                         Glib::SPAWN_DEFAULT,
                         sigc::mem_fun(this, &SystemInfoHardware::set_env),
                         &cmd_output);
    }
    catch (const Glib::Error& e)
    {
        LOG_WARNING("%s", e.what().c_str());
        return Json::Value();
    }

    cmd_output = StrUtils::trim(cmd_output);

    // lshw命令返回的json字符串是一个数组格式，但是没有用[]括起来，导致解析存在问题，这里对返回值进行修复
    if (add_bracket && cmd_output.length() > 1 && cmd_output[0] == '{')
    {
        if (cmd_output.back() == ',')
        {
            cmd_output.pop_back();
        }
        cmd_output.insert(cmd_output.begin(), '[');
        cmd_output.push_back(']');
    }

    Json::Value values;
    Json::CharReaderBuilder builder;
    std::string error;
    builder["collectComments"] = false;
    if (!builder.newCharReader()->parse(cmd_output.data(), cmd_output.data() + cmd_output.size(), &values, &error))
    {
        LOG_WARNING("%s", error.c_str());
        return Json::Value();
    }

    return values;
}

}  // namespace Kiran