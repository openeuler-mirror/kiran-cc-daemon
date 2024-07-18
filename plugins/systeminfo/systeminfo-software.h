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

#include "lib/base/base.h"

namespace Kiran
{
struct SoftwareInfo
{
    // 内核名称
    std::string kernel_name;
    // 主机名
    std::string host_name;
    // 内核发行号
    std::string kernel_release;
    // 内核版本
    std::string kernel_version;
    // 系统架构
    std::string arch;
    // 产品发行名称
    std::string product_name;
    // 产品发行版本
    std::string product_release;
};

class SystemInfoSoftware
{
public:
    SystemInfoSoftware();
    virtual ~SystemInfoSoftware(){};

    // 获取软件信息，数据来自uname函数和/etc/.kyinfo文件
    SoftwareInfo get_software_info();

    // 设置主机名
    bool set_host_name(const std::string &host_name);

private:
    bool read_kernel_info(SoftwareInfo &software_info);
    void read_product_info(SoftwareInfo &software_info);
};
}  // namespace Kiran