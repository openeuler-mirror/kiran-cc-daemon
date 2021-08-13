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