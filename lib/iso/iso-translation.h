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
    ISOTranslation(){};
    virtual ~ISOTranslation(){};

    static ISOTranslation *get_instance() { return instance_; };

    static void global_init();

    static void global_deinit();

public:
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
    void init();

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
    static ISOTranslation *instance_;

    // 格式：<code, name>
    std::map<std::string, std::string> countrys_;
    // 格式：<code, name>
    std::map<std::string, std::string> languages_;
};
}  // namespace Kiran