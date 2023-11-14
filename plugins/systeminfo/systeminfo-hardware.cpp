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

#include "plugins/systeminfo/systeminfo-hardware.h"

#include <gio/gunixinputstream.h>
#include <glibtop/mem.h>
#include <gudev/gudev.h>
#include <cinttypes>
#include <fstream>

namespace Kiran
{
#define CPUINFO_CMD "/usr/bin/lscpu"
#define CPUINFO_FILE "/proc/cpuinfo"
#define CPUINFO_KEY_DELIMITER ':'
#define CPUINFO_KEY_MODEL "model name"
//龙芯cpuinfo中为大写
#define CPUINFO_KEY_MODEL_LS "Model Name"
#define CPUINFO_KEY_PROCESSOR "processor"

#define MEMINFO_FILE "/proc/meminfo"
#define MEMINFO_KEY_DELIMITER ':'
#define MEMINFO_KEY_MEMTOTAL "MemTotal"

#define DISKINFO_CMD "/usr/bin/lsblk"

// 使用相对路径，避免使用绝对路径时因系统版本导致的错误
#define PCIINFO_CMD "lspci"
#define PCIINFO_KEY_DELIMITER ':'

SystemInfoHardware::SystemInfoHardware() : mem_size_lshw(0)
{
    init_meminfo_with_lshw();
}

void SystemInfoHardware::init_meminfo_with_lshw()
{
    // 使用工具lshw获取硬件信息返回结果可能耗时1s左右，为避免用户读取数据延迟在初始化过程中调用一次即可
    std::string action = std::string("/usr/sbin/lshw -json");
    std::vector<std::string> argv = Glib::shell_parse_argv(action);
    std::vector<Glib::ustring> envp;
    int standard_output = 0;

    Glib::spawn_async_with_pipes(Glib::ustring(),
                                 argv,
                                 envp,
                                 Glib::SPAWN_DO_NOT_REAP_CHILD,
                                 Glib::SlotSpawnChildSetup(),
                                 &this->child_pid_,
                                 nullptr,
                                 &standard_output,
                                 nullptr);

    this->out_io_channel_ = Glib::IOChannel::create_from_fd(standard_output);

    this->out_io_source_ = this->out_io_channel_->create_watch(Glib::IOCondition::IO_IN | Glib::IOCondition::IO_PRI);
    this->out_io_connection_ = this->out_io_source_->connect(sigc::bind(sigc::mem_fun(this, &SystemInfoHardware::on_lshw_output), this->out_io_channel_));
    this->out_io_source_->attach(Glib::MainContext::get_default());

    this->watch_child_connection_ = Glib::signal_child_watch().connect(sigc::mem_fun(this, &SystemInfoHardware::on_child_watch), this->child_pid_);
}

HardwareInfo SystemInfoHardware::get_hardware_info()
{
    HardwareInfo hardware_info;
    hardware_info.cpu_info = this->get_cpu_info();
    hardware_info.mem_info = this->get_mem_info();
    hardware_info.disks_info = this->get_disks_info();
    hardware_info.eths_info = this->get_eths_info();
    hardware_info.graphics_info = this->get_graphics_info();
    return hardware_info;
}

CPUInfo SystemInfoHardware::merge_cpu_infos(const std::vector<CPUInfo>& cpu_infos)
{
    CPUInfo cpu_info;
    for (auto& iter : cpu_infos)
    {
        if (cpu_info.model.empty())
        {
            cpu_info.model = iter.model;
        }
        if (cpu_info.cores_number == 0)
        {
            cpu_info.cores_number = iter.cores_number;
        }
    }
    return cpu_info;
}

CPUInfo SystemInfoHardware::get_cpu_info()
{
    std::vector<CPUInfo> cpu_infos;
    cpu_infos.push_back(this->get_cpu_info_by_cmd());
    cpu_infos.push_back(this->read_cpu_info_by_conf());
    return this->merge_cpu_infos(cpu_infos);
}

CPUInfo SystemInfoHardware::get_cpu_info_by_cmd()
{
    // 低版本不支持-J选项
    std::vector<std::string> argv{CPUINFO_CMD};

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
        KLOG_WARNING_SYSTEMINFO("%s", e.what().c_str());
        return CPUInfo();
    }

    auto kv_list = this->format_to_kv_list(cmd_output);
    RETURN_VAL_IF_TRUE(kv_list.size() == 0, CPUInfo());

    CPUInfo cpu_info;
    for (auto& iter : kv_list[0])
    {
        switch (shash(iter.first.c_str()))
        {
        case "CPU(s)"_hash:
            cpu_info.cores_number = strtol(iter.second.c_str(), NULL, 0);
            break;
        case "Model name"_hash:
            cpu_info.model = iter.second;
            break;
        default:
            break;
        }
    }

    return cpu_info;
}

CPUInfo SystemInfoHardware::read_cpu_info_by_conf()
{
    CPUInfo cpu_info;
    auto cpu_maps = this->parse_info_file(CPUINFO_FILE, CPUINFO_KEY_DELIMITER);
    // 适配龙芯架构
    if (cpu_info.model.empty())
    {
        cpu_info.model = cpu_maps[CPUINFO_KEY_MODEL_LS];
    }
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

    mem_info.total_size = this->get_memory_size_with_dmi();
    mem_info.available_size = this->get_memory_size_with_libgtop();

    if (mem_info.total_size == 0)
    {
        mem_info.total_size = this->get_memory_size_with_lshw();
        KLOG_DEBUG_SYSTEMINFO("Get total size with lshw:%ld.", mem_info.total_size);
    }

    if (mem_info.total_size == 0)
    {
        mem_info.total_size = mem_info.available_size;
        KLOG_DEBUG_SYSTEMINFO("Get total size with libgtop:%ld.", mem_info.total_size);
    }

    KLOG_DEBUG_SYSTEMINFO("Use total size is %ld, available size is %ld.", mem_info.total_size, mem_info.available_size);

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
        KLOG_WARNING_SYSTEMINFO("Failed to open file %s.", path.c_str());
        return std::map<std::string, std::string>();
    }

    while (!fs.eof())
    {
        fs.getline(buffer, BUFSIZ);

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
    //  老版本lsblk不支持-J选项，所以这里不使用json格式
    DiskInfoVec disks_info;
    std::vector<std::string> argv{DISKINFO_CMD, "-d", "-b", "-P", "-o", "NAME,TYPE,SIZE,MODEL,VENDOR"};

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
        KLOG_WARNING_SYSTEMINFO("%s", e.what().c_str());
        return disks_info;
    }

    auto lines = StrUtils::split_lines(cmd_output);

    for (auto& line : lines)
    {
        char name[BUFSIZ] = {0};
        char type[BUFSIZ] = {0};
        int64_t size = 0;
        char model[BUFSIZ] = {0};
        char vendor[BUFSIZ] = {0};

        std::string formatstr = "NAME=\"%" + std::to_string(BUFSIZ - 1) + "[^\"]\" TYPE=\"%" + std::to_string(BUFSIZ - 1) + "[^\"]\" SIZE=\"%" + PRId64 +
                                "\" MODEL=\"%" + std::to_string(BUFSIZ - 1) + "[^\"]\" VENDOR=\"%" + std::to_string(BUFSIZ - 1) + "[^\"]\"";

        // model和vendor只要有一个不为空则认为合法，所以只要能读到4个变量即可
        //"NAME=\"%[^\"]\" TYPE=\"%[^\"]\" SIZE=\"%" PRId64 "\" MODEL=\"%[^\"]\" VENDOR=\"%[^\"]\""
        if (sscanf(line.c_str(), formatstr.c_str(),
                   name, type, &size, model, vendor) >= 4)
        {
            if (std::string(type) == "disk")
            {
                DiskInfo disk_info;
                disk_info.name = name;
                disk_info.size = size;
                disk_info.model = StrUtils::trim(model);
                disk_info.vendor = StrUtils::trim(vendor);
                disks_info.push_back(disk_info);
            }
        }
    }

    return disks_info;
}

EthInfoVec SystemInfoHardware::get_eths_info()
{
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

KVList SystemInfoHardware::get_pcis_by_major_class_id(PCIMajorClassID major_class_id)
{
    std::vector<unsigned int> full_class_ids;

    // 获取主类ID为major_class_id的full_class_id列表，例如major_class_id为02，full_class_id列表为[0201, 0202]
    {
        std::string cmd_output;
        std::vector<std::string> argv{PCIINFO_CMD, "-n"};
        try
        {
            Glib::spawn_sync("",
                             argv,
                             Glib::SPAWN_SEARCH_PATH,
                             sigc::mem_fun(this, &SystemInfoHardware::set_env),
                             &cmd_output);
        }
        catch (const Glib::Error& e)
        {
            KLOG_WARNING_SYSTEMINFO("%s", e.what().c_str());
            return KVList();
        }

        auto lines = StrUtils::split_lines(cmd_output);
        for (auto& line : lines)
        {
            char placehold1[10];
            int32_t full_class_id;
            if (sscanf(line.c_str(), "%9s %x:", placehold1, &full_class_id) == 2)
            {
                if ((full_class_id >> 8) == major_class_id)
                {
                    full_class_ids.push_back(full_class_id);
                }
            }
        }
    }

    // 如果为空则不执行下面的命令，否则会取到所有的PCI设备(没有了-d选项的限制)
    RETURN_VAL_IF_TRUE(full_class_ids.size() == 0, KVList());

    // 根据full_class_id列表获取设备相关信息
    std::string full_outputs;
    for (auto& full_class_id : full_class_ids)
    {
        std::string cmd_output;
        std::vector<std::string> argv{PCIINFO_CMD, "-vmm"};
        argv.push_back("-d");
        argv.push_back(fmt::format("::{:04x}", full_class_id));

        KLOG_DEBUG_SYSTEMINFO("Cmdline: %s.", StrUtils::join(argv, " ").c_str());
        try
        {
            Glib::spawn_sync("",
                             argv,
                             Glib::SPAWN_SEARCH_PATH,
                             sigc::mem_fun(this, &SystemInfoHardware::set_env),
                             &cmd_output);
        }
        catch (const Glib::Error& e)
        {
            KLOG_WARNING_SYSTEMINFO("%s", e.what().c_str());
            return KVList();
        }

        full_outputs.append(cmd_output);
    }

    if (full_outputs.empty())
    {
        KLOG_WARNING_SYSTEMINFO("Get empty pci info calss id:%d.", major_class_id);
        return KVList();
    }

    return this->format_to_kv_list(full_outputs);
}

KVList SystemInfoHardware::format_to_kv_list(const std::string& contents)
{
    KVList pcis_info;
    auto regex = Glib::Regex::create("\n\n");
    std::vector<Glib::ustring> blocks = regex->split(contents, Glib::REGEX_MATCH_NEWLINE_ANY);
    for (auto& block : blocks)
    {
        std::map<std::string, std::string> pci_info;
        auto lines = StrUtils::split_lines(block);
        for (auto& line : lines)
        {
            auto fields = StrUtils::split_once_with_char(line, PCIINFO_KEY_DELIMITER);
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

void SystemInfoHardware::on_child_watch(GPid pid, int child_status)
{
    if (WIFEXITED(child_status))
    {
        if (WEXITSTATUS(child_status) >= 255)
        {
            KLOG_WARNING_SYSTEMINFO("Child exited unexpectedly");
        }
        else
        {
            this->parse_lshw_memory_info();
        }
    }
    else
    {
        KLOG_WARNING_SYSTEMINFO("Child exited error");
    }

    this->watch_child_connection_.disconnect();

    if (this->out_io_source_)
    {
        this->out_io_source_->destroy();
    }

    if (this->child_pid_)
    {
        Glib::spawn_close_pid(this->child_pid_);
        this->child_pid_ = 0;
    }

    this->out_io_connection_.disconnect();
    this->out_io_channel_.reset();
}

bool SystemInfoHardware::on_lshw_output(Glib::IOCondition io_condition, Glib::RefPtr<Glib::IOChannel> io_channel)
{
    try
    {
        Glib::ustring channel_info;
        auto retval = io_channel->read_to_end(channel_info);
        if (retval != Glib::IO_STATUS_NORMAL)
        {
            KLOG_WARNING_SYSTEMINFO("Failed to read data from IO channel. retval: %d.", retval);
        }
        else
        {
            this->hardware_info_lshw.append(channel_info);
        }
    }
    catch (const Glib::Error& e)
    {
        KLOG_WARNING_SYSTEMINFO("IO Channel read error: %s.", e.what().c_str());
    }

    return true;
}

void SystemInfoHardware::parse_lshw_memory_info()
{
    Json::Reader reader;
    Json::Value root;

    if (!reader.parse(this->hardware_info_lshw.c_str(), root))
    {
        KLOG_WARNING_SYSTEMINFO("Failed to parse lshw info size:%d.", this->hardware_info_lshw.size());
        return;
    }

    try
    {
        std::string class_val;
        std::string desc_val;

        for (unsigned int i = 0; i < root["children"].size(); i++)
        {
            Json::Value children = root["children"][i];
            for (unsigned int j = 0; j < children["children"].size(); j++)
            {
                class_val = children["children"][j]["class"].asString();
                desc_val = children["children"][j]["description"].asString();
                if (class_val == "memory" && (desc_val == "System memory" || desc_val == "System Memory"))
                {
                    this->mem_size_lshw = children["children"][j]["size"].asInt64();
                    KLOG_DEBUG_SYSTEMINFO("Find System memory size:%ld", this->mem_size_lshw);
                    break;
                }
            }
        }
    }
    catch (const Glib::Error& e)
    {
        KLOG_WARNING_SYSTEMINFO("%s.", e.what().c_str());
    }

    return;
}

int64_t SystemInfoHardware::get_memory_size_with_lshw()
{
    return this->mem_size_lshw;
}

int64_t SystemInfoHardware::get_memory_size_with_libgtop()
{
    glibtop_mem mem;
    glibtop_get_mem(&mem);

    return mem.total;
}

int64_t SystemInfoHardware::get_memory_size_with_dmi()
{
    g_autoptr(GUdevClient) client = NULL;
    g_autoptr(GUdevDevice) dmi = NULL;
    const gchar* const subsystems[] = {"dmi", NULL};
    uint64_t ram_total = 0;
    uint64_t num_ram = 0;

    client = g_udev_client_new(subsystems);
    dmi = g_udev_client_query_by_sysfs_path(client, "/sys/devices/virtual/dmi/id");
    if (!dmi)
    {
        KLOG_WARNING_SYSTEMINFO("Get dmi failed.");
        return 0;
    }

    num_ram = g_udev_device_get_property_as_uint64(dmi, "MEMORY_ARRAY_NUM_DEVICES");
    for (uint64_t i = 0; i < num_ram; i++)
    {
        std::string prop = fmt::format("MEMORY_DEVICE_{0}_SIZE", i);
        ram_total += g_udev_device_get_property_as_uint64(dmi, prop.c_str());
    }

    return ram_total;
}

}  // namespace Kiran