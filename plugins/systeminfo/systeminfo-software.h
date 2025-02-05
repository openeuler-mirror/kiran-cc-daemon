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
#pragma once

#include <QObject>

namespace Kiran
{
struct SoftwareInfo
{
    // 内核名称
    QString kernelName;
    // 主机名
    QString hostName;
    // 内核发行号
    QString kernelRelease;
    // 内核版本
    QString kernelVersion;
    // 系统架构
    QString arch;
    // 产品发行名称
    QString productName;
    // 产品发行版本
    QString productRelease;
};

class SystemInfoSoftware : public QObject
{
    Q_OBJECT

public:
    SystemInfoSoftware(QObject *parent = nullptr);
    virtual ~SystemInfoSoftware(){};

    // 获取软件信息，数据来自uname函数和/etc/.kyinfo文件
    SoftwareInfo getSoftwareInfo();

    // 设置主机名
    bool setHostName(const QString &hostName);

private:
    bool readKernelInfo(SoftwareInfo &softwareInfo);
    void readProductInfo(SoftwareInfo &softwareInfo);
    QString getReleaseInfo(const QString &program, const QStringList &arguments);
};
}  // namespace Kiran