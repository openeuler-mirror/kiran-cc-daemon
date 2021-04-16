/**
 * @file          /kiran-cc-daemon/plugins/audio/audio-utils.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#include "lib/base/base.h"

namespace Kiran
{
class AudioUtils
{
public:
    AudioUtils(){};
    virtual ~AudioUtils(){};

    // 音量范围转绝对值
    static uint32_t volume_range2absolute(double range,
                                          uint32_t min_volume,
                                          uint32_t max_volume);

    static double volume_absolute2range(uint32_t absolute,
                                        uint32_t min_volume,
                                        uint32_t max_volume);
};
}  // namespace Kiran