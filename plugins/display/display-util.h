
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

    static std::string rotation_to_str(DisplayRotationType rotation);
    static std::string reflect_to_str(DisplayReflectType reflect);
    static DisplayRotationType str_to_rotation(const std::string &str);
    static DisplayReflectType str_to_reflect(const std::string &str);
};
}  // namespace Kiran