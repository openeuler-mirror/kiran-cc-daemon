/**
 * @Copyright (C) 2020 ~ 2021 KylinSec Co., Ltd. 
 *
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; If not, see <http: //www.gnu.org/licenses/>. 
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