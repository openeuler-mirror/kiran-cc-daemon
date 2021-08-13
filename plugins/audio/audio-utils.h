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