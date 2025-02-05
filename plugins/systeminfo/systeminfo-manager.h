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

#include <QDBusContext>

class SystemInfoAdaptor;

namespace Kiran
{

class SystemInfoSoftware;
class SystemInfoHardware;

class SystemInfoManager : public QObject,
                          protected QDBusContext
{
    Q_OBJECT

    Q_PROPERTY(QString host_name READ getHostName WRITE setHostName)
public:
    SystemInfoManager();
    virtual ~SystemInfoManager(){};

    static SystemInfoManager* getInstance() { return m_instance; };
    static void globalInit();
    static void globalDeinit() { delete m_instance; };

public:
    QString getHostName() const;
    void setHostName(const QString& hostName);

public Q_SLOTS:
    /**
     * @brief 获取系统信息，如果type为SYSTEMINFO_TYPE_SOFTWARE则返回SoftwareInfo结构体中的内容，
     * 如果type为SYSTEMINFO_TYPE_SOFTWARE，则返回HardwareInfo结构体中的内容。
     * @param {type} 类型请查看SystemInfoType
     * @return 返回json格式的字符串。
     */
    QString GetSystemInfo(int type);
    /**
     * @brief 设置主机名
     * @param {host_name} 主机名
     * @return 如果调用sethostname函数失败，则返回错误，否则返回成功。
     */
    void SetHostName(const QString& hostName);

private:
    void init();

    void processHostNameChanged(const QDBusMessage& message, const QString& hostName);

private:
    static SystemInfoManager* m_instance;

    SystemInfoAdaptor* m_adaptor;
    SystemInfoSoftware* m_software;
    SystemInfoHardware* m_hardware;
};
}  // namespace Kiran