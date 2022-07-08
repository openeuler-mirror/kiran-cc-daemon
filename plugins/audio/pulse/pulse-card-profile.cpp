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

#include "plugins/audio/pulse/pulse-card-profile.h"

namespace Kiran
{
PulseCardProfile::PulseCardProfile(const pa_card_profile_info2 *card_profile_info) : name_(POINTER_TO_STRING(card_profile_info->name)),
                                                                                     description_(POINTER_TO_STRING(card_profile_info->description)),
                                                                                     n_sinks_(card_profile_info->n_sinks),
                                                                                     n_sources_(card_profile_info->n_sources),
                                                                                     priority_(card_profile_info->priority)
{
}
}  // namespace Kiran