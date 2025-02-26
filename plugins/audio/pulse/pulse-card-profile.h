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

#include <pulse/introspect.h>
#include <QObject>
#include <QString>

namespace Kiran
{
class PulseCardProfile : public QObject
{
    Q_OBJECT

public:
    PulseCardProfile(const pa_card_profile_info2 *cardProfileInfo);
    virtual ~PulseCardProfile(){};

    const QString &getName() const { return m_name; };
    const QString &getDescription() const { return m_description; };
    uint32_t getSinkCount() const { return m_nSinks; };
    uint32_t getSourceCount() const { return m_nSources; };
    uint32_t getPriority() const { return m_priority; };

private:
    // card profile名字
    QString m_name;
    // card profile描述
    QString m_description;
    // sink数量
    uint32_t m_nSinks;
    // source数量
    uint32_t m_nSources;
    // 优先级越高，表示越适合作为默认profile
    uint32_t m_priority;
};

}  // namespace Kiran