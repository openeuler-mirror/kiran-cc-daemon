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
#include <QSharedPointer>

namespace Kiran
{
class PowerSession;
// 计算机节能
class PowerSaveComputer : public QObject
{
    Q_OBJECT

public:
    PowerSaveComputer(QObject *parent = nullptr);
    virtual ~PowerSaveComputer(){};

    void init();

    // 挂机
    void suspend();
    // 休眠
    void hibernate();
    // 关机
    void shutdown();

private:
    QSharedPointer<PowerSession> m_session;
};
}  // namespace  Kiran
