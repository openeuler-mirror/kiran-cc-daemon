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
