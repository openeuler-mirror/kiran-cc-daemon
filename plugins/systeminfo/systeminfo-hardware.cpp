/**
 * @file          /kiran-cc-daemon/plugins/systeminfo/systeminfo-hardware.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#include "plugins/systeminfo/systeminfo-hardware.h"

#include <json/json.h>

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

#define PCIINFO_CMD "/usr/sbin/lspci"
#define PCIINFO_KEY_DELIMITER ':'

SystemInfoHardware::SystemInfoHardware()
{
}

HardwareInfo SystemInfoHardware::get_hardware_info()
{
    SETTINGS_PROFILE("");

    HardwareInfo hardware_info;
    this->read_cpu_info(hardware_info.cpu_info);
    this->read_mem_info(hardware_info.mem_info);
    this->read_disks_info(hardware_info.disks_info);
    this->read_eths_info(hardware_info.eths_info);
    this->read_graphics_info(hardware_info.graphics_info);
    return hardware_info;
}

void SystemInfoHardware::read_cpu_info(CPUInfo& cpu_info)
{
    RETURN_IF_TRUE(this->read_cpu_info_by_cmd(cpu_info));

    this->read_cpu_info_by_conf(cpu_info);
}

bool SystemInfoHardware::read_cpu_info_by_cmd(CPUInfo& cpu_info)
{
    std::string cmd_output;
    try
    {
        std::vector<std::string> argv{CPUINFO_CMD, "-J"};

        Glib::spawn_sync("",
                         argv,
                         Glib::SPAWN_DEFAULT,
                         sigc::mem_fun(this, &SystemInfoHardware::set_env),
                         &cmd_output);
    }
    catch (const Glib::Error& e)
    {
        LOG_WARNING("%s", e.what().c_str());
        return false;
    }

    Json::Value values;
    Json::CharReaderBuilder builder;
    std::string error;
    builder["collectComments"] = false;
    if (!builder.newCharReader()->parse(cmd_output.data(), cmd_output.data() + cmd_output.size(), &values, &error))
    {
        LOG_WARNING("%s", error.c_str());
        return false;
    }

    try
    {
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
        return false;
    }
    return true;
}

void SystemInfoHardware::read_cpu_info_by_conf(CPUInfo& cpu_info)
{
    auto cpu_maps = this->parse_info_file(CPUINFO_FILE, CPUINFO_KEY_DELIMITER);

    cpu_info.model = cpu_maps[CPUINFO_KEY_MODEL];
    if (cpu_maps.find(CPUINFO_KEY_PROCESSOR) != cpu_maps.end())
    {
        cpu_info.cores_number = strtol(cpu_maps[CPUINFO_KEY_PROCESSOR].c_str(), NULL, 0) + 1;
    }
}

void SystemInfoHardware::read_mem_info(MemInfo& mem_info)
{
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

void SystemInfoHardware::read_disks_info(DiskInfoVec& disks_info)
{
    std::string cmd_output;
    try
    {
        std::vector<std::string> argv{DISKINFO_CMD, "-d", "-b", "-J", "-o", "NAME,TYPE,SIZE,MODEL,VENDOR"};

        Glib::spawn_sync("",
                         argv,
                         Glib::SPAWN_DEFAULT,
                         sigc::mem_fun(this, &SystemInfoHardware::set_env),
                         &cmd_output);
    }
    catch (const Glib::Error& e)
    {
        LOG_WARNING("%s", e.what().c_str());
        return;
    }

    Json::Value values;
    Json::CharReaderBuilder builder;
    std::string error;
    builder["collectComments"] = false;
    if (!builder.newCharReader()->parse(cmd_output.data(), cmd_output.data() + cmd_output.size(), &values, &error))
    {
        LOG_WARNING("%s", error.c_str());
        return;
    }

    try
    {
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
        return;
    }
}

void SystemInfoHardware::read_eths_info(EthInfoVec& eths_info)
{
    // 部分机型的网卡可能是USB接口，但lsusb无法通过类型来区分那些是网卡，因此这里先不做处理。

    std::string cmd_output;
    try
    {
        std::vector<std::string> argv{PCIINFO_CMD, "-vmm"};

        Glib::spawn_sync("",
                         argv,
                         Glib::SPAWN_DEFAULT,
                         sigc::mem_fun(this, &SystemInfoHardware::set_env),
                         &cmd_output);
    }
    catch (const Glib::Error& e)
    {
        LOG_WARNING("%s", e.what().c_str());
        return;
    }

    auto pcis_info = this->parse_pcis_info(cmd_output);

    for (auto& pci_info : pcis_info)
    {
        EthInfo eth_info;
        auto regex = Glib::Regex::create("Ethernet.*controller", Glib::REGEX_CASELESS);
        if (!regex->match(pci_info["Class"]))
        {
            continue;
        }
        eth_info.model = pci_info["Device"];
        eth_info.vendor = pci_info["Vendor"];
        eths_info.push_back(std::move(eth_info));
    }
}

void SystemInfoHardware::read_graphics_info(GraphicInfoVec& graphics_info)
{
    std::string cmd_output;
    try
    {
        std::vector<std::string> argv{PCIINFO_CMD, "-vmm"};

        Glib::spawn_sync("",
                         argv,
                         Glib::SPAWN_DEFAULT,
                         sigc::mem_fun(this, &SystemInfoHardware::set_env),
                         &cmd_output);
    }
    catch (const Glib::Error& e)
    {
        LOG_WARNING("%s", e.what().c_str());
        return;
    }

    auto pcis_info = this->parse_pcis_info(cmd_output);

    for (auto& pci_info : pcis_info)
    {
        GraphicInfo graphic_info;
        auto regex = Glib::Regex::create("VGA.*controller", Glib::REGEX_CASELESS);
        if (!regex->match(pci_info["Class"]))
        {
            continue;
        }
        graphic_info.model = pci_info["Device"];
        graphic_info.vendor = pci_info["Vendor"];
        graphics_info.push_back(std::move(graphic_info));
    }
}

PCIsInfo SystemInfoHardware::parse_pcis_info(const std::string& contents)
{
    PCIsInfo pcis_info;
    auto regex = Glib::Regex::create("\n\n");
    std::vector<Glib::ustring> blocks = regex->split(contents, Glib::REGEX_MATCH_NEWLINE_ANY);
    for (auto& block : blocks)
    {
        std::map<std::string, std::string> pci_info;
        auto lines = StrUtils::split_lines(block);
        for (auto& line : lines)
        {
            auto fields = StrUtils::split_with_char(line, PCIINFO_KEY_DELIMITER);
            if (fields.size() != 2)
            {
                continue;
            }
            pci_info[StrUtils::trim(fields[0])] = StrUtils::trim(fields[1]);
        }
        pcis_info.push_back(std::move(pci_info));
    }
    return pcis_info;
}

}  // namespace Kiran