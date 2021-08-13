/**
 * @Copyright (C) 2020 ~ 2021 KylinSec Co., Ltd. 
 *
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; If not, see <http: //www.gnu.org/licenses/>. 
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
    bool read_product_info(SoftwareInfo &software_info);
};
}  // namespace Kiran