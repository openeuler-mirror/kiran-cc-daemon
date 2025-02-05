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

#include "pulse-card-profile.h"
#include "lib/base/base.h"

namespace Kiran
{
PulseCardProfile::PulseCardProfile(const pa_card_profile_info2 *cardProfileInfo) : m_name(POINTER_TO_STRING(cardProfileInfo->name)),
                                                                                   m_description(POINTER_TO_STRING(cardProfileInfo->description)),
                                                                                   m_nSinks(cardProfileInfo->n_sinks),
                                                                                   m_nSources(cardProfileInfo->n_sources),
                                                                                   m_priority(cardProfileInfo->priority)
{
}
}  // namespace Kiran