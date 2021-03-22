/**
 * @file          /kiran-menu/home/tangjie02/git/kiran-cc-daemon/plugins/systeminfo/systeminfo-software.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
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
    SETTINGS_PROFILE("");

    SoftwareInfo software_info;
    this->read_kernel_info(software_info);
    this->read_product_info(software_info);
    return software_info;
}

bool SystemInfoSoftware::set_host_name(const std::string &host_name)
{
    SETTINGS_PROFILE("host name: %s.", host_name.c_str());

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
        LOG_WARNING("%s", e.what().c_str());
        return false;
    }

    return true;
}

bool SystemInfoSoftware::read_kernel_info(SoftwareInfo &software_info)
{
    SETTINGS_PROFILE("");

    struct utsname uts_name;

    auto retval = uname(&uts_name);
    if (retval < 0)
    {
        LOG_WARNING("call uname() failed: %s.", strerror(errno));
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
    SETTINGS_PROFILE("");

    Glib::KeyFile keyfile;

    try
    {
        keyfile.load_from_file(KYINFO_FILE);

        software_info.product_name = keyfile.get_string(KYINFO_GROUP_NAME, KYINFO_KEY_NAME);
        software_info.product_release = keyfile.get_string(KYINFO_GROUP_NAME, KYINFO_KEY_MILESTONE);
    }
    catch (const Glib::Error &e)
    {
        LOG_WARNING("%s", e.what().c_str());
        return false;
    }

    return true;
}
}  // namespace Kiran