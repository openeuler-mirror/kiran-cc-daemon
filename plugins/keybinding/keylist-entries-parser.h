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

#pragma once

#include <map>
#include <memory>
#include <vector>

namespace xmlpp
{
class Node;
};

struct KeyListEntry
{
    // gsettings的键名(key)；值为按键组合，例如<ctrl>O
    std::string name;
    // 按键组合描述
    std::string description;
    // 如果schema:key  comparison value 成立，则entry可用
    std::string schema;
    std::string key;
    std::string value;
    std::string comparison;
};

struct KeyListEntries
{
    // gsettings路径
    std::string schema;
    // domainname，可以翻译description字段中的内容
    std::string package;
    // 按键绑定所属分类
    std::string name;
    // 窗口管理器按键绑定组名
    std::string wm_name;
    // 具体的按键绑定列表
    std::vector<KeyListEntry> entries_;
};

namespace Kiran
{
// 这里是为了兼容mate-control-center中已有的按键绑定配置
class KeyListEntriesParser
{
public:
    KeyListEntriesParser(const std::string &key_entry_dir);
    virtual ~KeyListEntriesParser(){};

    bool parse(std::vector<KeyListEntries> &keys, std::string &err);

private:
    bool process_keylist_entries(const xmlpp::Node *node, KeyListEntries &keylist_entries, std::string &err);
    bool process_keylist_entry(const xmlpp::Node *node, KeyListEntry &keylist_entry, std::string &err);

private:
    std::string key_entry_dir_;
};

}  // namespace Kiran
