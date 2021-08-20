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

#include "plugins/power/power-utils.h"

#include <fmt/format.h>
#include <glib/gi18n.h>

namespace Kiran
{
std::string PowerUtils::get_time_translation(uint32_t seconds)
{
    auto minutes = seconds / 60;

    RETURN_VAL_IF_TRUE(minutes == 0, _("Less than 1 minute"));

    if (minutes < 60)
    {
        return fmt::format(ngettext("{0} minute", "{0} minutes", minutes), minutes);
    }

    auto hours = minutes / 60;
    minutes = minutes % 60;

    if (minutes == 0)
    {
        return fmt::format(ngettext("{0} hour", "{0} hours", hours), hours);
    }
    else
    {
        return fmt::format("{0} {1} {2} {3}",
                           hours,
                           ngettext("hour", "hours", hours),
                           minutes,
                           ngettext("minute", "minutes", minutes));
    }
}
}  // namespace Kiran