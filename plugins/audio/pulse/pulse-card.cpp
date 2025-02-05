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

#include "pulse-card.h"
#include "lib/base/base.h"
#include "pulse-card-port.h"
#include "pulse-card-profile.h"
#include "pulse-stream.h"

namespace Kiran
{
PulseCard::PulseCard(const pa_card_info *cardInfo) : m_index(cardInfo->index),
                                                     m_name(POINTER_TO_STRING(cardInfo->name))
{
    load(cardInfo);
}

void PulseCard::update(const pa_card_info *cardInfo)
{
    RETURN_IF_FALSE(cardInfo != NULL);

    QString activeProfileName;

    // 只有active profile可能会发生变化，因此其他属性不进行更新
    if (cardInfo->active_profile2 != NULL)
    {
        activeProfileName = POINTER_TO_STRING(cardInfo->active_profile2->name);
    }

    if (m_activeProfileName != activeProfileName)
    {
        m_activeProfileName = activeProfileName;
        auto activeProfile = getProfile(m_activeProfileName);
        Q_EMIT activeProfileChanged(activeProfile);
    }
}

void PulseCard::load(const pa_card_info *cardInfo)
{
    RETURN_IF_FALSE(cardInfo != NULL);

    m_cardPorts.clear();
    m_cardProfiles.clear();
    m_activeProfileName.clear();

    for (uint32_t i = 0; i < cardInfo->n_ports; ++i)
    {
        auto cardPort = QSharedPointer<PulseCardPort>::create(cardInfo->ports[i]);
        auto portName = cardPort->getName();
        if (m_cardPorts.contains(portName))
        {
            KLOG_WARNING(audio) << "The port" << portName << "is already exist.";
        }
        else
        {
            m_cardPorts.insert(portName, cardPort);
        }
    }

    for (uint32_t i = 0; i < cardInfo->n_profiles; ++i)
    {
        auto card_profile = QSharedPointer<PulseCardProfile>::create(cardInfo->profiles2[i]);
        auto profileName = card_profile->getName();
        if (m_cardProfiles.contains(profileName))
        {
            KLOG_WARNING(audio) << "The profile" << profileName << "is already exist.";
        }
        else
        {
            m_cardProfiles.insert(profileName, card_profile);
        }
    }

    if (cardInfo->active_profile2 != NULL)
    {
        m_activeProfileName = POINTER_TO_STRING(cardInfo->active_profile2->name);
    }
}

PulseCard::~PulseCard()
{
}
}  // namespace Kiran