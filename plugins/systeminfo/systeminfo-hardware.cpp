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

#define PCIINFO_CMD "/usr/sbin/lspci"
#define PCIINFO_KEY_DELIMITER ':'

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
    // 老版本lsblk不支持-J选项，所以这里不使用json格式
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
}

EthInfoVec SystemInfoHardware::get_eths_info()
{
    SETTINGS_PROFILE("");

    EthInfoVec eths_info;
    auto pcis_info = this->get_pcis_by_major_class_id(PCIMajorClassID::PCI_MAJOR_CLASS_ID_NETWORK);

    for (auto& pci_info : pcis_info)
    {
        EthInfo eth_info;
        eth_info.model = pci_info["Device"];
        eth_info.vendor = pci_info["Vendor"];
        eths_info.push_back(std::move(eth_info));
    }
    return eths_info;
}

GraphicInfoVec SystemInfoHardware::get_graphics_info()
{
    SETTINGS_PROFILE("");

    GraphicInfoVec graphics_info;
    auto pcis_info = this->get_pcis_by_major_class_id(PCIMajorClassID::PCI_MAJOR_CLASS_ID_DISPLAY);

    for (auto& pci_info : pcis_info)
    {
        GraphicInfo graphic_info;
        graphic_info.model = pci_info["Device"];
        graphic_info.vendor = pci_info["Vendor"];
        graphics_info.push_back(std::move(graphic_info));
    }
    return graphics_info;
}

PCIsInfo SystemInfoHardware::get_pcis_by_major_class_id(PCIMajorClassID major_class_id)
{
    std::vector<int32_t> full_class_ids;

    // 获取主类ID为major_class_id的full_class_id列表，例如major_class_id为02，full_class_id列表为[0201, 0202]
    {
        std::string cmd_output;
        std::vector<std::string> argv{PCIINFO_CMD, "-n"};
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
            return PCIsInfo();
        }

        auto lines = StrUtils::split_lines(cmd_output);
        for (auto& line : lines)
        {
            char placehold1[10];
            int32_t full_class_id;
            if (sscanf(line.c_str(), "%s %x:", placehold1, &full_class_id) == 2)
            {
                if ((full_class_id >> 8) == major_class_id)
                {
                    full_class_ids.push_back(full_class_id);
                }
            }
        }
    }

    // 根据full_class_id列表获取设备相关信息
    {
        std::string cmd_output;
        std::vector<std::string> argv{PCIINFO_CMD, "-vmm"};

        for (auto& full_class_id : full_class_ids)
        {
            argv.push_back("-d");
            argv.push_back(fmt::format("::{:04x}", full_class_id));
        }

        LOG_DEBUG("cmdline: %s.", StrUtils::join(argv, " ").c_str());
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
            return PCIsInfo();
        }
        return this->parse_pcis_output(cmd_output);
    }
}

PCIsInfo SystemInfoHardware::parse_pcis_output(const std::string& contents)
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
        if (pci_info.size() > 0)
        {
            pcis_info.push_back(std::move(pci_info));
        }
    }
    return pcis_info;
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