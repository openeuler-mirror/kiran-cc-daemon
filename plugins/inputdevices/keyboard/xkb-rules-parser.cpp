/*
 * @Author       : tangjie02
 * @Date         : 2020-08-18 09:47:39
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-08-26 13:57:38
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/plugins/inputdevices/keyboard/xkb-rules-parser.cpp
 */

#include "plugins/inputdevices/keyboard/xkb-rules-parser.h"

#include <libxml++/libxml++.h>

#include "lib/log.h"

namespace Kiran
{
XkbRulesParser::XkbRulesParser(const std::string &file_name) : file_name_(file_name)
{
}

bool XkbRulesParser::parse(XkbRules &xkb_rules, std::string &err)
{
    SETTINGS_PROFILE("");
    try
    {
        xmlpp::DomParser parser;
        parser.set_throw_messages(true);
        parser.parse_file(this->file_name_);
        if (parser)
        {
            const auto root_node = parser.get_document()->get_root_node();
            RETURN_VAL_IF_FALSE(this->process_config_registry(root_node, xkb_rules, err), false);
        }
    }
    catch (const std::exception &e)
    {
        err = e.what();
        return false;
    }
    return true;
}

bool XkbRulesParser::process_config_registry(const xmlpp::Node *node, XkbRules &xkb_rules, std::string &err)
{
    SETTINGS_PROFILE("node_name: %s.", node ? node->get_name().c_str() : "null");
    CHECK_XMLPP_ELEMENT(node, err);

    for (const auto &child : node->get_children())
    {
        switch (shash(child->get_name().c_str()))
        {
        case "modelList"_hash:
            RETURN_VAL_IF_FALSE(this->process_models(child, xkb_rules.models, err), false);
            break;
        case "layoutList"_hash:
            RETURN_VAL_IF_FALSE(this->process_layouts(child, xkb_rules.layouts, err), false);
            break;
        case "optionList"_hash:
            RETURN_VAL_IF_FALSE(this->process_option_groups(child, xkb_rules.option_groups, err), false);
            break;
        case "text"_hash:
            // no deal
            break;
        default:
            LOG_DEBUG("ignore node: %s.", child->get_name().c_str());
            break;
        }
    }
    return true;
}

bool XkbRulesParser::process_models(const xmlpp::Node *node, std::vector<XkbModel> &xkb_models, std::string &err)
{
    CHECK_XMLPP_ELEMENT(node, err);

    for (const auto &child : node->get_children())
    {
        switch (shash(child->get_name().c_str()))
        {
        case "model"_hash:
        {
            XkbModel xkb_model;
            RETURN_VAL_IF_FALSE(this->process_model(child, xkb_model, err), false);
            xkb_models.push_back(std::move(xkb_model));
        }
        break;
        case "text"_hash:
            break;
        default:
            LOG_DEBUG("ignore node: %s.", child->get_name().c_str());
            break;
        }
    }
    return true;
}

bool XkbRulesParser::process_model(const xmlpp::Node *node, XkbModel &xkb_model, std::string &err)
{
    CHECK_XMLPP_ELEMENT(node, err);

    for (const auto &child : node->get_children())
    {
        switch (shash(child->get_name().c_str()))
        {
        case "configItem"_hash:
            RETURN_VAL_IF_FALSE(this->process_model_config_item(child, xkb_model, err), false);
            break;
        case "text"_hash:
            break;
        default:
            LOG_DEBUG("ignore node: %s.", child->get_name().c_str());
            break;
        }
    }
    return true;
}

bool XkbRulesParser::process_model_config_item(const xmlpp::Node *node, XkbModel &xkb_model, std::string &err)
{
    CHECK_XMLPP_ELEMENT(node, err);

    for (const auto &child : node->get_children())
    {
        switch (shash(child->get_name().c_str()))
        {
        case "name"_hash:
            RETURN_VAL_IF_FALSE(this->process_content_node(child, xkb_model.name, err), false);
            break;
        case "description"_hash:
            RETURN_VAL_IF_FALSE(this->process_content_node(child, xkb_model.description, err), false);
            break;
        case "vendor"_hash:
            RETURN_VAL_IF_FALSE(this->process_content_node(child, xkb_model.vendor, err), false);
            break;
        case "text"_hash:
            break;
        default:
            LOG_DEBUG("ignore node: %s.", child->get_name().c_str());
            break;
        }
    }
    return true;
}

bool XkbRulesParser::process_layouts(const xmlpp::Node *node, std::vector<XkbLayout> &xkb_layouts, std::string &err)
{
    CHECK_XMLPP_ELEMENT(node, err);

    for (const auto &child : node->get_children())
    {
        switch (shash(child->get_name().c_str()))
        {
        case "layout"_hash:
        {
            XkbLayout xkb_layout;
            RETURN_VAL_IF_FALSE(this->process_layout(child, xkb_layout, err), false);
            xkb_layouts.push_back(std::move(xkb_layout));
        }
        break;
        case "text"_hash:
            break;
        default:
            LOG_DEBUG("ignore node: %s.", child->get_name().c_str());
            break;
        }
    }
    return true;
}

bool XkbRulesParser::process_layout(const xmlpp::Node *node, XkbLayout &xkb_layout, std::string &err)
{
    CHECK_XMLPP_ELEMENT(node, err);

    for (const auto &child : node->get_children())
    {
        switch (shash(child->get_name().c_str()))
        {
        case "configItem"_hash:
            RETURN_VAL_IF_FALSE(this->process_layout_config_item(child, xkb_layout, err), false);
            break;
        case "variantList"_hash:
            RETURN_VAL_IF_FALSE(this->process_layout_variants(child, xkb_layout.variants, err), false);
            break;
        case "text"_hash:
            break;
        default:
            LOG_DEBUG("ignore node: %s.", child->get_name().c_str());
            break;
        }
    }
    return true;
}

bool XkbRulesParser::process_layout_config_item(const xmlpp::Node *node, XkbLayout &xkb_layout, std::string &err)
{
    CHECK_XMLPP_ELEMENT(node, err);

    for (const auto &child : node->get_children())
    {
        switch (shash(child->get_name().c_str()))
        {
        case "name"_hash:
            RETURN_VAL_IF_FALSE(this->process_content_node(child, xkb_layout.name, err), false);
            break;
        case "shortDescription"_hash:
            RETURN_VAL_IF_FALSE(this->process_content_node(child, xkb_layout.short_description, err), false);
            break;
        case "description"_hash:
            RETURN_VAL_IF_FALSE(this->process_content_node(child, xkb_layout.description, err), false);
            break;
        case "languageList"_hash:
        case "countryList"_hash:
        case "text"_hash:
            break;
        default:
            LOG_DEBUG("ignore node: %s.", child->get_name().c_str());
            break;
        }
    }
    return true;
}

bool XkbRulesParser::process_layout_variants(const xmlpp::Node *node, std::vector<XkbVariant> &xkb_variants, std::string &err)
{
    CHECK_XMLPP_ELEMENT(node, err);

    for (const auto &child : node->get_children())
    {
        switch (shash(child->get_name().c_str()))
        {
        case "variant"_hash:
        {
            XkbVariant xkb_variant;
            RETURN_VAL_IF_FALSE(this->process_layout_variant(child, xkb_variant, err), false);
            xkb_variants.push_back(std::move(xkb_variant));
        }
        break;
        case "text"_hash:
            break;
        default:
            LOG_DEBUG("ignore node: %s.", child->get_name().c_str());
            break;
        }
    }
    return true;
}

bool XkbRulesParser::process_layout_variant(const xmlpp::Node *node, XkbVariant &xkb_variant, std::string &err)
{
    CHECK_XMLPP_ELEMENT(node, err);

    for (const auto &child : node->get_children())
    {
        switch (shash(child->get_name().c_str()))
        {
        case "configItem"_hash:
            RETURN_VAL_IF_FALSE(this->process_layout_variant_config_item(child, xkb_variant, err), false);
            break;
        case "text"_hash:
            break;
        default:
            LOG_DEBUG("ignore node: %s.", child->get_name().c_str());
            break;
        }
    }
    return true;
}

bool XkbRulesParser::process_layout_variant_config_item(const xmlpp::Node *node, XkbVariant &xkb_variant, std::string &err)
{
    CHECK_XMLPP_ELEMENT(node, err);

    for (const auto &child : node->get_children())
    {
        switch (shash(child->get_name().c_str()))
        {
        case "name"_hash:
            RETURN_VAL_IF_FALSE(this->process_content_node(child, xkb_variant.name, err), false);
            break;
        case "shortDescription"_hash:
            RETURN_VAL_IF_FALSE(this->process_content_node(child, xkb_variant.short_description, err), false);
            break;
        case "description"_hash:
            RETURN_VAL_IF_FALSE(this->process_content_node(child, xkb_variant.description, err), false);
            break;
        case "text"_hash:
        case "languageList"_hash:
            break;
        default:
            LOG_DEBUG("ignore node: %s.", child->get_name().c_str());
            break;
        }
    }
    return true;
}

bool XkbRulesParser::process_option_groups(const xmlpp::Node *node, std::vector<XkbOptionGroup> &xkb_option_groups, std::string &err)
{
    CHECK_XMLPP_ELEMENT(node, err);

    for (const auto &child : node->get_children())
    {
        switch (shash(child->get_name().c_str()))
        {
        case "group"_hash:
        {
            XkbOptionGroup xkb_option_group;
            RETURN_VAL_IF_FALSE(this->process_option_group(child, xkb_option_group, err), false);
            xkb_option_groups.push_back(std::move(xkb_option_group));
        }
        break;
        case "text"_hash:
            break;
        default:
            LOG_DEBUG("ignore node: %s.", child->get_name().c_str());
            break;
        }
    }
    return true;
}

bool XkbRulesParser::process_option_group(const xmlpp::Node *node, XkbOptionGroup &xkb_option_group, std::string &err)
{
    CHECK_XMLPP_ELEMENT(node, err);

    for (const auto &child : node->get_children())
    {
        switch (shash(child->get_name().c_str()))
        {
        case "configItem"_hash:
            RETURN_VAL_IF_FALSE(this->process_option_group_config_item(child, xkb_option_group, err), false);
            break;
        case "option"_hash:
        {
            XkbOption xkb_option;
            RETURN_VAL_IF_FALSE(this->process_option(child, xkb_option, err), false);
            xkb_option_group.options.push_back(std::move(xkb_option));
        }
        break;
        case "text"_hash:
            break;
        default:
            LOG_DEBUG("ignore node: %s.", child->get_name().c_str());
            break;
        }
    }
    return true;
}

bool XkbRulesParser::process_option_group_config_item(const xmlpp::Node *node, XkbOptionGroup &xkb_option_group, std::string &err)
{
    CHECK_XMLPP_ELEMENT(node, err);

    for (const auto &child : node->get_children())
    {
        switch (shash(child->get_name().c_str()))
        {
        case "name"_hash:
            RETURN_VAL_IF_FALSE(this->process_content_node(child, xkb_option_group.name, err), false);
            break;
        case "description"_hash:
            RETURN_VAL_IF_FALSE(this->process_content_node(child, xkb_option_group.description, err), false);
            break;
        case "text"_hash:
            break;
        default:
            LOG_DEBUG("ignore node: %s.", child->get_name().c_str());
            break;
        }
    }
    return true;
}

bool XkbRulesParser::process_option(const xmlpp::Node *node, XkbOption &xkb_option, std::string &err)
{
    CHECK_XMLPP_ELEMENT(node, err);

    for (const auto &child : node->get_children())
    {
        switch (shash(child->get_name().c_str()))
        {
        case "configItem"_hash:
            RETURN_VAL_IF_FALSE(this->process_option_config_item(child, xkb_option, err), false);
            break;
        case "text"_hash:
            break;
        default:
            LOG_DEBUG("ignore node: %s.", child->get_name().c_str());
            break;
        }
    }
    return true;
}

bool XkbRulesParser::process_option_config_item(const xmlpp::Node *node, XkbOption &xkb_option, std::string &err)
{
    CHECK_XMLPP_ELEMENT(node, err);

    for (const auto &child : node->get_children())
    {
        switch (shash(child->get_name().c_str()))
        {
        case "name"_hash:
            RETURN_VAL_IF_FALSE(this->process_content_node(child, xkb_option.name, err), false);
            break;
        case "description"_hash:
            RETURN_VAL_IF_FALSE(this->process_content_node(child, xkb_option.description, err), false);
            break;
        case "text"_hash:
            break;
        default:
            LOG_DEBUG("ignore node: %s.", child->get_name().c_str());
            break;
        }
    }
    return true;
}

bool XkbRulesParser::process_content_node(const xmlpp::Node *node, std::string &content, std::string &err)
{
    const auto element = dynamic_cast<const xmlpp::Element *>(node);

    if (!element)
    {
        err = fmt::format("the type of the node '{0}' isn't xmlpp::Element.", node->get_name().c_str());
        return false;
    }

    auto text_node = element->get_child_text();

    if (!text_node)
    {
        err = fmt::format("the node '{0}' hasn't xmlpp::TextNode.", node->get_name().c_str());
        return false;
    }

    // LOG_DEBUG("node: %s text_node: %s.", node->get_name().c_str(), text_node->get_name().c_str());

    content = text_node->get_content().raw();
    return true;
}
}  // namespace  Kiran
