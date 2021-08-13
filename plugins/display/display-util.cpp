
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

#include "plugins/display/display-util.h"

namespace Kiran
{
std::string DisplayUtil::rotation_to_str(DisplayRotationType rotation)
{
    switch (rotation)
    {
    case DisplayRotationType::DISPLAY_ROTATION_90:
        return "left";
    case DisplayRotationType::DISPLAY_ROTATION_180:
        return "inverted";
    case DisplayRotationType::DISPLAY_ROTATION_270:
        return "right";
    default:
        return "normal";
    }
}

std::string DisplayUtil::reflect_to_str(DisplayReflectType reflect)
{
    switch (reflect)
    {
    case DisplayReflectType::DISPLAY_REFLECT_X:
        return "x";
    case DisplayReflectType::DISPLAY_REFLECT_Y:
        return "y";
    case DisplayReflectType::DISPLAY_REFLECT_XY:
        return "xy";
    default:
        return "normal";
        break;
    }
}

DisplayRotationType DisplayUtil::str_to_rotation(const std::string &str)
{
    switch (shash(str.c_str()))
    {
    case "left"_hash:
        return DisplayRotationType::DISPLAY_ROTATION_90;
    case "inverted"_hash:
        return DisplayRotationType::DISPLAY_ROTATION_180;
    case "right"_hash:
        return DisplayRotationType::DISPLAY_ROTATION_270;
    default:
        return DisplayRotationType::DISPLAY_ROTATION_0;
    }
    return DisplayRotationType::DISPLAY_ROTATION_0;
}

DisplayReflectType DisplayUtil::str_to_reflect(const std::string &str)
{
    switch (shash(str.c_str()))
    {
    case "x"_hash:
        return DisplayReflectType::DISPLAY_REFLECT_X;
    case "y"_hash:
        return DisplayReflectType::DISPLAY_REFLECT_Y;
    case "xy"_hash:
        return DisplayReflectType::DISPLAY_REFLECT_XY;
    default:
        return DisplayReflectType::DISPLAY_REFLECT_NORMAL;
    }
    return DisplayReflectType::DISPLAY_REFLECT_NORMAL;
}
}  // namespace Kiran
