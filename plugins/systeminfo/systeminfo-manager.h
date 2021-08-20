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

#include <systeminfo_dbus_stub.h>

#include "plugins/systeminfo/systeminfo-hardware.h"
#include "plugins/systeminfo/systeminfo-software.h"

namespace Kiran
{
class SystemInfoManager : public SystemDaemon::SystemInfoStub
{
public:
    SystemInfoManager();
    virtual ~SystemInfoManager(){};

    static SystemInfoManager* get_instance() { return instance_; };

    static void global_init();

    static void global_deinit() { delete instance_; };

protected:
    /**
     * @brief 获取系统信息，如果type为SYSTEMINFO_TYPE_SOFTWARE则返回SoftwareInfo结构体中的内容，
     * 如果type为SYSTEMINFO_TYPE_SOFTWARE，则返回HardwareInfo结构体中的内容。
     * @param {type} 类型请查看SystemInfoType
     * @return 返回json格式的字符串。
     */
    virtual void GetSystemInfo(gint32 type, MethodInvocation& invocation);

    /**
     * @brief 设置主机名
     * @param {host_name} 主机名
     * @return 如果调用sethostname函数失败，则返回错误，否则返回成功。
     */
    virtual void SetHostName(const Glib::ustring& host_name, MethodInvocation& invocation);

    virtual bool host_name_setHandler(const Glib::ustring& value);
    virtual Glib::ustring host_name_get();

private:
    void init();

    void change_host_name_cb(MethodInvocation invocation, const std::string& host_name);

    void on_bus_acquired(const Glib::RefPtr<Gio::DBus::Connection>& connect, Glib::ustring name);
    void on_name_acquired(const Glib::RefPtr<Gio::DBus::Connection>& connect, Glib::ustring name);
    void on_name_lost(const Glib::RefPtr<Gio::DBus::Connection>& connect, Glib::ustring name);

private:
    static SystemInfoManager* instance_;

    uint32_t dbus_connect_id_;
    uint32_t object_register_id_;

    SystemInfoSoftware software_;
    SystemInfoHardware hardware_;
};
}  // namespace Kiran