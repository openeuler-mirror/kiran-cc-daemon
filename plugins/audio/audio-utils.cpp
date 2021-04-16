/**
 * @file          /kiran-cc-daemon/plugins/audio/audio-utils.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#include "plugins/audio/audio-utils.h"

namespace Kiran
{
uint32_t AudioUtils::volume_range2absolute(double range,
                                           uint32_t min_volume,
                                           uint32_t max_volume)
{
    RETURN_VAL_IF_TRUE(range <= 0, min_volume);
    RETURN_VAL_IF_TRUE(range >= 1, max_volume);
    return uint32_t((max_volume - min_volume) * range) + min_volume;
}

double AudioUtils::volume_absolute2range(uint32_t absolute,
                                         uint32_t min_volume,
                                         uint32_t max_volume)
{
    RETURN_VAL_IF_TRUE(absolute > max_volume, 1.0);
    RETURN_VAL_IF_TRUE(absolute < min_volume, 0);
    return (absolute - min_volume) * 1.0 / (max_volume - min_volume);
}
}  // namespace  Kiran
