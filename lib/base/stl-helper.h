/**
 * @file          /kiran-cc-daemon/lib/base/stl-helper.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
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