/*
 * @Author       : tangjie02
 * @Date         : 2020-08-26 11:52:40
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-09-02 11:34:35
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/plugins/keybinding/keylist-entries-parser.h
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
