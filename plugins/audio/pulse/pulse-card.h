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
#include <QMap>
#include <QObject>
#include <QSharedPointer>

namespace Kiran
{

class PulseCardProfile;
class PulseCardPort;
class PulseNode;

class PulseCard : public QObject
{
    Q_OBJECT

public:
    PulseCard(const pa_card_info *card_info);
    virtual ~PulseCard();

    void update(const pa_card_info *card_info);

    uint32_t getIndex() const { return m_index; };
    const QString &getName() const { return m_name; };

    QSharedPointer<PulseCardProfile> getProfile(const QString &name) const { return m_cardProfiles.value(name); };
    QSharedPointer<PulseCardProfile> getActiveProfile() const { return getProfile(m_activeProfileName); };
    QSharedPointer<PulseCardPort> getCardPort(const QString &name) const { return m_cardPorts.value(name); };

Q_SIGNALS:
    void activeProfileChanged(QSharedPointer<PulseCardProfile> profile);

private:
    void load(const pa_card_info *cardInfo);

private:
    uint32_t m_index;
    QString m_name;

    QString m_activeProfileName;
    QMap<QString, QSharedPointer<PulseCardProfile>> m_cardProfiles;
    QMap<QString, QSharedPointer<PulseCardPort>> m_cardPorts;
    QMap<QString, QSharedPointer<PulseNode>> m_streams;
};

using PulseCardVec = QList<QSharedPointer<PulseCard>>;
}  // namespace  Kiran
