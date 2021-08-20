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

#include <string>
#include <vector>

namespace xmlpp
{
class Node;
};

namespace Kiran
{
struct XkbModel
{
    std::string name;
    std::string description;
    std::string vendor;
};

struct XkbVariant
{
    std::string name;
    std::string short_description;
    std::string description;
};

struct XkbLayout
{
    std::string name;
    std::string short_description;
    std::string description;
    std::vector<XkbVariant> variants;
};

struct XkbOption
{
    std::string name;
    std::string description;
};

struct XkbOptionGroup
{
    std::string name;
    std::string description;
    std::vector<XkbOption> options;
};

struct XkbRules
{
    std::vector<XkbModel> models;
    std::vector<XkbLayout> layouts;
    std::vector<XkbOptionGroup> option_groups;
};

class XkbRulesParser
{
public:
    XkbRulesParser(const std::string &file_name);
    virtual ~XkbRulesParser(){};

    bool parse(XkbRules &xkb_rules, std::string &err);

private:
    bool process_config_registry(const xmlpp::Node *node, XkbRules &xkb_rules, std::string &err);

    bool process_models(const xmlpp::Node *node, std::vector<XkbModel> &xkb_models, std::string &err);
    bool process_model(const xmlpp::Node *node, XkbModel &xkb_model, std::string &err);
    bool process_model_config_item(const xmlpp::Node *node, XkbModel &xkb_model, std::string &err);

    bool process_layouts(const xmlpp::Node *node, std::vector<XkbLayout> &xkb_layouts, std::string &err);
    bool process_layout(const xmlpp::Node *node, XkbLayout &xkb_layout, std::string &err);
    bool process_layout_config_item(const xmlpp::Node *node, XkbLayout &xkb_layout, std::string &err);
    bool process_layout_variants(const xmlpp::Node *node, std::vector<XkbVariant> &xkb_variants, std::string &err);
    bool process_layout_variant(const xmlpp::Node *node, XkbVariant &xkb_variant, std::string &err);
    bool process_layout_variant_config_item(const xmlpp::Node *node, XkbVariant &xkb_variant, std::string &err);

    bool process_option_groups(const xmlpp::Node *node, std::vector<XkbOptionGroup> &xkb_option_groups, std::string &err);
    bool process_option_group(const xmlpp::Node *node, XkbOptionGroup &xkb_option_group, std::string &err);
    bool process_option_group_config_item(const xmlpp::Node *node, XkbOptionGroup &xkb_option_group, std::string &err);
    bool process_option(const xmlpp::Node *node, XkbOption &xkb_option, std::string &err);
    bool process_option_config_item(const xmlpp::Node *node, XkbOption &xkb_option, std::string &err);

    bool process_content_node(const xmlpp::Node *node, std::string &content, std::string &err);

private:
    std::string file_name_;
};
}  // namespace Kiran