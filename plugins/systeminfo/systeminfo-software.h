/**
 * @file          /kiran-menu/home/tangjie02/git/kiran-cc-daemon/plugins/systeminfo/systeminfo-software.h
 * @brief         系统内核相关信息
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
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