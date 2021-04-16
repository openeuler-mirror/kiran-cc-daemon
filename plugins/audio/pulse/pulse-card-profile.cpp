/**
 * @file          /kiran-cc-daemon/plugins/audio/pulse/pulse-card-profile.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
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