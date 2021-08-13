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


#include <map>
#include <vector>

namespace Kiran
{
class MapHelper
{
public:
    MapHelper(){};
    virtual ~MapHelper(){};

    template <typename K, typename V>
    static std::vector<V> get_values(const std::map<K, V> &maps)
    {
        std::vector<V> values;
        for (auto &iter : maps)
        {
            values.push_back(iter.second);
        }
        return values;
    }

    template <typename K, typename V>
    static V get_value(const std::map<K, V> &maps, const K &key)
    {
        auto iter = maps.find(key);
        if (iter != maps.end())
        {
            return iter->second;
        }
        return nullptr;
    }
};

}  // namespace Kiran