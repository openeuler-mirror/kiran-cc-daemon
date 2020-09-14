
#pragma once

#include <string>

#include "plugins/display/xrandr-manager.h"

namespace Kiran
{
class DisplayUtil
{
public:
    DisplayUtil(){};
    virtual ~DisplayUtil(){};

    static std::string rotation_to_str(RotationType rotation);
    static std::string reflect_to_str(ReflectType reflect);
    static RotationType str_to_rotation(const std::string &str);
    static ReflectType str_to_reflect(const std::string &str);
};
}  // namespace Kiran