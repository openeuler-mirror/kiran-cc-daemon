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

#include "plugins/systeminfo/systeminfo-software.h"

#include <sys/utsname.h>

namespace Kiran
{
#define KYINFO_FILE "/etc/.kyinfo"
#define KYINFO_GROUP_NAME "dist"
#define KYINFO_KEY_NAME "name"
#define KYINFO_KEY_MILESTONE "milestone"

#define SET_HOSTNAME_CMD "/usr/bin/hostnamectl"

SystemInfoSoftware::SystemInfoSoftware()
{
}

SoftwareInfo SystemInfoSoftware::get_software_info()
{
    KLOG_PROFILE("");

    SoftwareInfo software_info;
    this->read_kernel_info(software_info);
    this->read_product_info(software_info);
    return software_info;
}

bool SystemInfoSoftware::set_host_name(const std::string &host_name)
{
    KLOG_PROFILE("host name: %s.", host_name.c_str());

    std::vector<std::string> argv{SET_HOSTNAME_CMD, "set-hostname", host_name};

    std::string cmd_output;
    try
    {
        Glib::spawn_sync("",
                         argv,
                         Glib::SPAWN_DEFAULT,
                         Glib::SlotSpawnChildSetup(),
                         nullptr);
    }
    catch (const Glib::Error &e)
    {
        KLOG_WARNING("%s", e.what().c_str());
        return false;
    }

    return true;
}

bool SystemInfoSoftware::read_kernel_info(SoftwareInfo &software_info)
{
    KLOG_PROFILE("");

    struct utsname uts_name;

    auto retval = uname(&uts_name);
    if (retval < 0)
    {
        KLOG_WARNING("call uname() failed: %s.", strerror(errno));
        return false;
    }

    software_info.kernel_name = uts_name.sysname;
    software_info.host_name = uts_name.nodename;
    software_info.kernel_release = uts_name.release;
    software_info.kernel_version = uts_name.version;
    software_info.arch = uts_name.machine;

    return true;
}

bool SystemInfoSoftware::read_product_info(SoftwareInfo &software_info)
{
    KLOG_PROFILE("");

    Glib::KeyFile keyfile;

    try
    {
        keyfile.load_from_file(KYINFO_FILE);

        software_info.product_name = keyfile.get_string(KYINFO_GROUP_NAME, KYINFO_KEY_NAME);
        software_info.product_release = keyfile.get_string(KYINFO_GROUP_NAME, KYINFO_KEY_MILESTONE);
    }
    catch (const Glib::Error &e)
    {
        KLOG_WARNING("%s", e.what().c_str());
        return false;
    }

    return true;
}
}  // namespace Kiran