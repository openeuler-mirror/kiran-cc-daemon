/**
 * @file          /kiran-cc-daemon/plugins/systeminfo/systeminfo-manager.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#include "plugins/systeminfo/systeminfo-manager.h"

#include <json/json.h>

#include "lib/dbus/dbus.h"
#include "systeminfo_i.h"

namespace Kiran
{
#define AUTH_SET_HOST_NAME "com.kylinsec.kiran.system-daemon.systeminfo.set-host-name"
SystemInfoManager::SystemInfoManager() : dbus_connect_id_(0),
                                         object_register_id_(0)
{
}

SystemInfoManager* SystemInfoManager::instance_ = nullptr;
void SystemInfoManager::global_init()
{
    instance_ = new SystemInfoManager();
    instance_->init();
}

void SystemInfoManager::GetSystemInfo(gint32 type, MethodInvocation& invocation)
{
    Json::Value values;
    Json::FastWriter writer;

    try
    {
        switch (type)
        {
        case SystemInfoType::SYSTEMINFO_TYPE_SOFTWARE:
        {
            auto software_info = this->software_.get_software_info();
            values["kernal_name"] = software_info.kernel_name;
            values["host_name"] = software_info.host_name;
            values["kernel_release"] = software_info.kernel_release;
            values["kernel_version"] = software_info.kernel_version;
            values["arch"] = software_info.arch;
            values["product_name"] = software_info.product_name;
            values["product_release"] = software_info.product_release;

            break;
        }
        case SystemInfoType::SYSTEMINFO_TYPE_HARDWARE:
        {
            auto hardware_info = this->hardware_.get_hardware_info();
            values["cpu"]["model"] = hardware_info.cpu_info.model;
            values["cpu"]["cores_number"] = hardware_info.cpu_info.cores_number;
            values["mem"]["total_size"] = Json::Int64(hardware_info.mem_info.total_size);
            for (uint32_t i = 0; i < hardware_info.disks_info.size(); ++i)
            {
                Json::Value disk_value;
                disk_value["name"] = hardware_info.disks_info[i].name;
                disk_value["model"] = hardware_info.disks_info[i].model;
                disk_value["vendor"] = hardware_info.disks_info[i].vendor;
                disk_value["size"] = Json::Int64(hardware_info.disks_info[i].size);
                values["disks"].append(disk_value);
            }

            for (uint32_t i = 0; i < hardware_info.eths_info.size(); ++i)
            {
                Json::Value eth_value;
                eth_value["model"] = hardware_info.eths_info[i].model;
                eth_value["vendor"] = hardware_info.eths_info[i].vendor;
                values["eths"].append(eth_value);
            }

            for (uint32_t i = 0; i < hardware_info.graphics_info.size(); ++i)
            {
                Json::Value graphic_value;
                graphic_value["model"] = hardware_info.graphics_info[i].model;
                graphic_value["vendor"] = hardware_info.graphics_info[i].vendor;
                values["graphics"].append(graphic_value);
            }
            break;
        }
        default:
            DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_SYSTEMINFO_TYPE_INVALID);
        }
    }
    catch (const std::exception& e)
    {
        LOG_WARNING("%s.", e.what());
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_SYSTEMINFO_JSON_ASSIGN_FAILED);
    }

    auto result = writer.write(values);
    invocation.ret(result);
}

void SystemInfoManager::SetHostName(const Glib::ustring& host_name, MethodInvocation& invocation)
{
    SETTINGS_PROFILE("host name: %s", host_name.c_str());

    AuthManager::get_instance()->start_auth_check(AUTH_SET_HOST_NAME,
                                                  true,
                                                  invocation.getMessage(),
                                                  std::bind(&SystemInfoManager::change_host_name_cb, this, std::placeholders::_1, host_name));
}

bool SystemInfoManager::host_name_setHandler(const Glib::ustring& value)
{
    return this->software_.set_host_name(value.raw());
}

Glib::ustring SystemInfoManager::host_name_get()
{
    auto software_info = this->software_.get_software_info();
    return software_info.host_name;
}

void SystemInfoManager::init()
{
    this->dbus_connect_id_ = Gio::DBus::own_name(Gio::DBus::BUS_TYPE_SYSTEM,
                                                 SYSTEMINFO_DBUS_NAME,
                                                 sigc::mem_fun(this, &SystemInfoManager::on_bus_acquired),
                                                 sigc::mem_fun(this, &SystemInfoManager::on_name_acquired),
                                                 sigc::mem_fun(this, &SystemInfoManager::on_name_lost));
}

void SystemInfoManager::change_host_name_cb(MethodInvocation invocation, const std::string& host_name)
{
    SETTINGS_PROFILE("host name: %s", host_name.c_str());

    if (!this->host_name_set(host_name))
    {
        DBUS_ERROR_REPLY_AND_RET(CCErrorCode::ERROR_SYSTEMINFO_SET_HOSTNAME_FAILED);
    }
    invocation.ret();
}

void SystemInfoManager::on_bus_acquired(const Glib::RefPtr<Gio::DBus::Connection>& connect, Glib::ustring name)
{
    SETTINGS_PROFILE("name: %s", name.c_str());
    if (!connect)
    {
        LOG_WARNING("Failed to connect dbus. name: %s", name.c_str());
        return;
    }
    try
    {
        this->object_register_id_ = this->register_object(connect, SYSTEMINFO_OBJECT_PATH);
    }
    catch (const Glib::Error& e)
    {
        LOG_WARNING("register object_path %s fail: %s.", SYSTEMINFO_OBJECT_PATH, e.what().c_str());
    }
}

void SystemInfoManager::on_name_acquired(const Glib::RefPtr<Gio::DBus::Connection>& connect, Glib::ustring name)
{
    LOG_DEBUG("success to register dbus name: %s", name.c_str());
}

void SystemInfoManager::on_name_lost(const Glib::RefPtr<Gio::DBus::Connection>& connect, Glib::ustring name)
{
    LOG_WARNING("failed to register dbus name: %s", name.c_str());
}
}  // namespace Kiran