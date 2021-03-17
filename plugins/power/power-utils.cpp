/**
 * @file          /kiran-cc-daemon/plugins/power/power-utils.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
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