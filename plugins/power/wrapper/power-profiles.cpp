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

#include "plugins/power/wrapper/power-profiles.h"
#include "plugins/power/wrapper/power-profiles-hadess.h"
#include "plugins/power/wrapper/power-profiles-tuned.h"
#include "power-i.h"

namespace Kiran
{
std::shared_ptr<PowerProfiles> PowerProfiles::create()
{
    auto power_settings = Gio::Settings::create(POWER_SCHEMA_ID);
    auto profiles_policy = power_settings->get_enum(POWER_SCHEMA_PROFILE_POLICY);
    switch (profiles_policy)
    {
    case PowerProfilePolicy::POWER_PROFILE_POLICY_HADESS:
        return std::make_shared<PowerProfilesHadess>();
    case PowerProfilePolicy::POWER_PROFILE_POLICY_TUNED:
        return std::make_shared<PowerProfilesTuned>();
    default:
        return std::make_shared<PowerProfilesTuned>();
    }
}
}  // namespace Kiran
