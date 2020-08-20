/*
 * @Author       : tangjie02
 * @Date         : 2020-08-20 11:49:32
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-08-20 15:26:13
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/plugins/inputdevices/keyboard/iso-translation.h
 */

#pragma once

#include <map>
#include <string>
#include <vector>

namespace xmlpp
{
class Node;
};

namespace Kiran
{
class ISOTranslation
{
public:
    ISOTranslation(const std::string &iso_dir);
    virtual ~ISOTranslation(){};

    // 通过code获得城市/地区的名字
    std::string get_country_name(const std::string &code);
    // 通过code获得城市/地区的名字，并对名字进行本地化
    std::string get_locale_country_name(const std::string &code);

    // 通过code获得语言名
    std::string get_language_name(const std::string &code);
    // 对字符串进行本地化
    std::string get_locale_string(const std::string &str, const std::string &delimeters);
    std::string get_locale_string(const std::string &str);

private:
    bool load_iso_file(const std::string &iso_basename,
                       const std::vector<std::string> &code_attr_names,
                       std::map<std::string, std::string> &result,
                       std::string &err);

    bool process_iso_entries(const xmlpp::Node *node,
                             const std::string &iso_basename,
                             const std::vector<std::string> &code_attr_names,
                             std::map<std::string, std::string> &result,
                             std::string &err);

    bool process_iso_entry(const xmlpp::Node *node,
                           const std::string &iso_basename,
                           const std::vector<std::string> &code_attr_names,
                           std::map<std::string, std::string> &result,
                           std::string &err);

private:
    std::string iso_dir_;
    // 格式：<code, name>
    std::map<std::string, std::string> countrys_;
    // 格式：<code, name>
    std::map<std::string, std::string> languages_;
};
}  // namespace Kiran