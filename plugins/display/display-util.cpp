
#include "plugins/display/display-util.h"

namespace Kiran
{
std::string DisplayUtil::rotation_to_str(RotationType rotation)
{
    switch (rotation)
    {
    case RotationType::ROTATION_90:
        return "left";
    case RotationType::ROTATION_180:
        return "inverted";
    case RotationType::ROTATION_270:
        return "right";
    default:
        return "normal";
    }
}

std::string DisplayUtil::reflect_to_str(ReflectType reflect)
{
    switch (reflect)
    {
    case ReflectType::REFLECT_X:
        return "x";
    case ReflectType::REFLECT_Y:
        return "y";
    case ReflectType::REFLECT_XY:
        return "xy";
    default:
        return "normal";
        break;
    }
}

RotationType DisplayUtil::str_to_rotation(const std::string &str)
{
    switch (shash(str.c_str()))
    {
    case "left"_hash:
        return RotationType::ROTATION_90;
    case "inverted"_hash:
        return RotationType::ROTATION_180;
    case "right"_hash:
        return RotationType::ROTATION_270;
    default:
        return RotationType::ROTATION_0;
    }
    return RotationType::ROTATION_0;
}

ReflectType DisplayUtil::str_to_reflect(const std::string &str)
{
    switch (shash(str.c_str()))
    {
    case "x"_hash:
        return ReflectType::REFLECT_X;
    case "y"_hash:
        return ReflectType::REFLECT_Y;
    case "xy"_hash:
        return ReflectType::REFLECT_XY;
    default:
        return ReflectType::REFLECT_NORMAL;
    }
    return ReflectType::REFLECT_NORMAL;
}
}  // namespace Kiran
